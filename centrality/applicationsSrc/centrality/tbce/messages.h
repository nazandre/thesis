/*
 * messages.h
 *
 *  Created on: 01/09/2016
 *      Author: Andre Naz
 */

#ifndef TBCE_MESSAGES_H_
#define TBCE_MESSAGES_H_

#include "../eaMsg.h"
#include "../utils/def.h"

#define TB_CENTRALITY_GO_LEADER_MESSAGE 1005
#define TB_CENTRALITY_BACK_LEADER_MESSAGE 1006
#define TB_CENTRALITY_LEADER_MESSAGE 1007

//===========================================================================================================
//
//          TBCentralityGoLeaderMsg  (class)
//
//===========================================================================================================

class TBCentralityGoLeaderMsg;
typedef std::shared_ptr<TBCentralityGoLeaderMsg>  TBCentralityGoLeaderMsg_ptr;

class TBCentralityGoLeaderMsg : public EAMessage {
 public :
  distance_t tbCentrality;
	
  TBCentralityGoLeaderMsg(distance_t tbc) {
    type = TB_CENTRALITY_GO_LEADER_MESSAGE;
    dataSize = sizeof(distance_t);
    tbCentrality = tbc;
  }
  ~TBCentralityGoLeaderMsg() {};
};


//===========================================================================================================
//
//          TBCentralityBackLeaderMsg  (class)
//
//===========================================================================================================

class TBCentralityBackLeaderMsg;
typedef std::shared_ptr<TBCentralityBackLeaderMsg>  TBCentralityBackLeaderMsg_ptr;

class TBCentralityBackLeaderMsg : public EAMessage {
 public :

  mID_t id;

  TBCentralityBackLeaderMsg( mID_t i) {
    type = TB_CENTRALITY_BACK_LEADER_MESSAGE;
    dataSize = sizeof(mID_t);
    id = i;
  }
  ~TBCentralityBackLeaderMsg() {};
};

//===========================================================================================================
//
//          TBCentralityLeaderMsg  (class)
//
//===========================================================================================================

class TBCentralityLeaderMsg;
typedef std::shared_ptr<TBCentralityLeaderMsg>  TBCentralityLeaderMsg_ptr;

class TBCentralityLeaderMsg : public EAMessage {
 public :
	
  mID_t id;
	
  TBCentralityLeaderMsg(mID_t i) {
    type = TB_CENTRALITY_LEADER_MESSAGE;
    dataSize = sizeof(mID_t);
    id = i;
  }
  ~TBCentralityLeaderMsg() {};
};

#endif
