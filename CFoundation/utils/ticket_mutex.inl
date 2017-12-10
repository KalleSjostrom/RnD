struct TicketMutex {
	int64_t volatile ticket;
	int64_t volatile serving;
};
__forceinline void aquire_ticket(TicketMutex &mutex) {
	int64_t ticket = _Thread.interlocked_exchange_add(&mutex.ticket, 1);
	while(ticket != mutex.serving);
}
__forceinline void release_ticket(TicketMutex &mutex) {
	_Thread.interlocked_exchange_add(&mutex.serving, 1);
}