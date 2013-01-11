SRC_CC = test_ipc.cc test_ipc_manager.cc ipc_call_queue.cc
LIBS   = thread

vpath test_ipc.cc $(REP_DIR)/src/base/ipc
vpath test_ipc_manager.cc $(REP_DIR)/src/base/ipc
vpath ipc_call_queue.cc $(REP_DIR)/src/base/ipc

