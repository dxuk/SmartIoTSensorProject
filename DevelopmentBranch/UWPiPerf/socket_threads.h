#ifndef SOCKET_THREADS_H
#define SOCKET_THREADS_H

extern void main_parameters_init();

extern void main_parameters_release();

extern HANDLE StartIPerfThread(int paramc, char ** params);

extern void EndIPerfThread(bool isServer);

#endif
