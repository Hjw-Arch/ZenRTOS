#include "rt_mailbox.h"


void mailBoxInit(mailBox_t* mailbox, void** messageBuffer, uint32_t maxcount) {
	eventInit(&mailbox->event, EVENT_TYPE_MAILBOX);
	
	mailbox->messageBuffer = messageBuffer;
	mailbox->readPos = 0;
	mailbox->writePos = 0;
	mailbox->counter = 0;
	mailbox->maxcount = maxcount;
}



