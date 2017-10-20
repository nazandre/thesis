#include "myMessage.h"
#include <iostream>

using namespace BaseSimulator;
using namespace std;

MyMessage::MyMessage(): Message() {
}

MyMessage::~MyMessage() {

}

MyMessage::size() {
  return MTU;
}
