SRC_CC = obtain_utcb.cc ipc.cc ipc_manager.cc ipc_message_queue.cc
LIBS   = thread

vpath obtain_utcb.cc       $(REP_DIR)/src/base/ipc
vpath ipc.cc               $(REP_DIR)/src/base/ipc
vpath ipc_manager.cc       $(REP_DIR)/src/base/ipc
vpath ipc_message_queue.cc $(REP_DIR)/src/base/ipc

