#ifndef BFS_DATA_H_
#define BFS_DATA_H_

class BFSData {
public:
  BFSData();
  virtual ~BFSData();

  virtual unsigned int size() = 0;

  static BFSData* getNULLBFSData();

  virtual BFSData* clone() = 0;
};

#endif
