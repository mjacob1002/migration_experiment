#include "iotest.decl.h"
#include <assert.h>
#include <iostream>
#include <fstream>
#include <time.h>
#include <vector>
#include <string>

#define PES_PER_NODE 2

CProxy_Main mainproxy;
std::string global_fname;

class Main : public CBase_Main
{
  Main_SDAG_CODE

  CProxy_Test testers;
  int n;

  Ck::IO::Session session;
  Ck::IO::File f;

  size_t fileSize;

  double start_time;
  int numBufChares;
  std::string filename;
  int num_maxes_received = 0;
  std::vector<double> _maxes;

public:
  Main(CkArgMsg* m)
  {
    numBufChares = atoi(m->argv[1]);  // arg 1 = number of buffer chares

    fileSize = atoi(m->argv[2]);  // file size = arg 2

    n = atoi(m->argv[3]);  // arg 3 = number of readers

    std::string fn(m->argv[4]);  // arg 4 = filename
    filename = fn;
    global_fname = fn;

    CkPrintf("Parsed args.\n");

    mainproxy = thisProxy;
    thisProxy.run();  // open files
    delete m;
  }

  void iterDone() { CkExit(); }

  void maxTime(double max_time){
  	CkPrintf("Inside max_time!\n");
  	std::string res_fname = global_fname + ".results";
  	std::ofstream ofs;
	ofs.open(res_fname, std::ios::app);
	_maxes.push_back(max_time);
	num_maxes_received++;
	CkPrintf("Number of maxes receieved: %d\n", num_maxes_received);
	if(num_maxes_received < 3){
		CkPrintf("issuing another read...\n");
		testers.issueRead();	
	} else {
		_maxes.erase(_maxes.begin());
		CkPrintf("writing the results to a file...\n");
		for(double max_double: _maxes){
			ofs << max_double << ",";
			CkPrintf("%f,", max_double);
		}
		ofs << std::endl;
		CkPrintf("finished writing results...\n");
		CkReductionMsg* msg = CkReductionMsg::buildNew(sizeof(double), &max_time);
		mainproxy.test_read(msg);
	}
  }
};

class Test : public CBase_Test
{
  char* dataBuffer;
  int size;
  size_t _bytesToRead;
  double begin_time;
  double end_time;
  Ck::IO::Session _token;
  double _diff;
  int num_reads_done = 0;

public:
  Test(Ck::IO::Session token, size_t bytesToRead, std::string filename)
  {
	_token = token;
	_bytesToRead = bytesToRead;
    thisProxy[thisIndex].testMethod(token, bytesToRead);
  }

  void testMethod(Ck::IO::Session token, size_t bytesToRead)
  {
    try
    {
      dataBuffer = new char[bytesToRead];
    }
    catch (const std::bad_alloc& e)
    {
      CkPrintf("ERROR: Data buffer malloc of %zu bytes in Test chare %d failed.\n",
               bytesToRead, thisIndex);
      CkExit();
    }

    // setup and read using Ck::IO::FileReader
    size = bytesToRead;
	begin_time = CkWallTimer();
    // read using plain Ck::IO::Read
    Ck::IO::read(token, bytesToRead, bytesToRead * thisIndex, dataBuffer, CkCallback(CkIndex_Test::readDone(0), thisProxy[thisIndex]));
  }

  void pup(PUP::er &p){
	p | num_reads_done;
	p | size;
	p | _bytesToRead;
	p | _diff;
  	if(p.isUnpacking()) {
		dataBuffer = new char[_bytesToRead];
	} 
	PUParray(p, dataBuffer, _bytesToRead);
	p | begin_time;
	p | end_time;
	p | _token;
  }

  Test(CkMigrateMessage* m) {

  }

 void issueRead(){
 	begin_time = CkWallTimer();
    Ck::IO::read(_token, _bytesToRead, _bytesToRead * thisIndex, dataBuffer, CkCallback(CkIndex_Test::readDone(0), thisProxy[thisIndex]));
 }

  void readDone(Ck::IO::ReadCompleteMsg* m){
  		end_time = CkWallTimer();
		_diff = end_time - begin_time;
		int dest_pe = (CkMyPe() + PES_PER_NODE) % CkNumPes();
		CkPrintf("destination PE=%d\n", dest_pe);
		num_reads_done++;
		if(num_reads_done == 1){
			contribute(sizeof(double), &_diff, CkReduction::max_double, CkCallback(CkReductionTarget(Main, maxTime), mainproxy));
		} else {
			migrateMe(dest_pe);
		}
	}

 void ckJustMigrated(){
 			CkPrintf("in the justMigrated method of %d with %d reads done and _diff=%f...\n", thisIndex, num_reads_done, _diff);
			thisProxy[thisIndex].fuckThis();

 }

 void fuckThis(){
 	CkPrintf("Fuck this entry method from %d on PE %d...\n", thisIndex, CkMyPe());
	contribute(sizeof(double), &_diff, CkReduction::max_double, CkCallback(CkReductionTarget(Main, maxTime), mainproxy));
		CkPrintf("just contributed from index=%d on PE=%d with %d reads done...\n", thisIndex, CkMyPe(), num_reads_done);
 }
};
#include "iotest.def.h"
