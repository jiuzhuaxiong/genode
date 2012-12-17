SRC_CC = ipc.cc ipc_manager.cc ipc_call_queue.cc
LIBS   = thread

vpath ipc.cc $(REP_DIR)/src/base/ipc
vpath ipc_manager.cc $(REP_DIR)/src/base/ipc
vpath ipc_call_queue.cc $(REP_DIR)/src/base/ipc

