// nnet/nnet-compute-lstm-lm-parallel.h

// Copyright 2015-2016   Shanghai Jiao Tong University (author: Wei Deng)

// See ../../COPYING for clarification regarding multiple authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
// THIS CODE IS PROVIDED *AS IS* BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION ANY IMPLIED
// WARRANTIES OR CONDITIONS OF TITLE, FITNESS FOR A PARTICULAR PURPOSE,
// MERCHANTABLITY OR NON-INFRINGEMENT.
// See the Apache 2 License for the specific language governing permissions and
// limitations under the License.

#ifndef KALDI_NNET_NNET_COMPUTE_LSTM_LM_PARALLEL_H_
#define KALDI_NNET_NNET_COMPUTE_LSTM_LM_PARALLEL_H_

#include "nnet2/am-nnet.h"
#include "hmm/transition-model.h"

#include <string>
#include <iomanip>
#include <mpi.h>

#include "nnet-trnopts.h"
#include "nnet/nnet-randomizer.h"
#include "nnet/nnet-loss.h"
#include "nnet/nnet-nnet.h"
#include "nnet/nnet-model-sync.h"

#include "cudamatrix/cu-device.h"

#include "nnet/nnet-compute-lstm-asgd.h"

namespace kaldi {
namespace nnet1 {

struct NnetLstmLmUpdateOptions : public NnetLstmUpdateOptions {

	std::string class_boundary;
	int32 num_class;

    NnetLstmLmUpdateOptions(const NnetTrainOptions *trn_opts, const NnetDataRandomizerOptions *rnd_opts, const NnetParallelOptions *parallel_opts)
    	: NnetLstmUpdateOptions(trn_opts, rnd_opts, parallel_opts), class_boundary(""), num_class(0) { }

  	  void Register(OptionsItf *po)
  	  {
  		  NnetLstmUpdateOptions::Register(po);

	      //lm
  		  po->Register("class-boundary", &class_boundary, "The fist index of each class(and final class class) in class based language model");
  		  po->Register("num-class", &num_class, "The number of class that the language model output");
  	  }
};


struct NnetLmStats: NnetStats {

    CBXent cbxent;

    NnetLmStats() { }

    void MergeStats(NnetUpdateOptions *opts, int root)
    {
        int myid = opts->parallel_opts->myid;
        MPI_Barrier(MPI_COMM_WORLD);

        void *addr = (void *) (myid==root ? MPI_IN_PLACE : (void*)(&this->total_frames));
        MPI_Reduce(addr, (void*)(&this->total_frames), 1, MPI_UNSIGNED_LONG, MPI_SUM, root, MPI_COMM_WORLD);

        addr = (void *) (myid==root ? MPI_IN_PLACE : (void*)(&this->num_done));
        MPI_Reduce(addr, (void*)(&this->num_done), 1, MPI_INT, MPI_SUM, root, MPI_COMM_WORLD);

        addr = (void *) (myid==root ? MPI_IN_PLACE : (void*)(&this->num_no_tgt_mat));
        MPI_Reduce(addr, (void*)(&this->num_no_tgt_mat), 1, MPI_INT, MPI_SUM, root, MPI_COMM_WORLD);

        addr = (void *) (myid==root ? MPI_IN_PLACE : (void*)(&this->num_other_error));
        MPI_Reduce(addr, (void*)(&this->num_other_error), 1, MPI_INT, MPI_SUM, root, MPI_COMM_WORLD);

        if (opts->objective_function == "xent") {
                        cbxent.Merge(myid, 0);
        }
        else {
        		KALDI_ERR << "Unknown objective function code : " << opts->objective_function;
        }

    }

    void Print(NnetUpdateOptions *opts, double time_now)
    {
        KALDI_LOG << "Done " << num_done << " files, " << num_no_tgt_mat
                  << " with no tgt_mats, " << num_other_error
                  << " with other errors. "
                  << "[" << (opts->crossvalidate?"CROSS-VALIDATION":"TRAINING")
                  << ", " << (opts->randomize?"RANDOMIZED":"NOT-RANDOMIZED")
                  << ", " << time_now/60 << " min, " << total_frames/time_now << " fps"
                  << "]";

        if (opts->objective_function == "xent") {
                KALDI_LOG << cbxent.Report();
        }
        else {
        	KALDI_ERR << "Unknown objective function code : " << opts->objective_function;
        }
    }
};

typedef struct Word_
{
	  int32  idx;
	  int32	 wordid;
	  int32  classid;
}Word;


void NnetLstmLmUpdateParallel(const NnetLstmUpdateOptions *opts,
		std::string	model_filename,
		std::string feature_rspecifier,
		Nnet *nnet,
		NnetLmStats *stats);


} // namespace nnet1
} // namespace kaldi

#endif // KALDI_NNET_NNET_COMPUTE_LSTM_H_
