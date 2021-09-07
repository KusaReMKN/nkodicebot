#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "mknmap.h"

struct score {
	long long score;
	unsigned long long dice;
};

static int open_connection(const char *host, const char *service);

void *KeyCopy(void *prev, const void *newkey)
{
	void *tmp;
	if (prev != NULL) return prev;
	tmp = malloc(strlen((const char *)newkey) + 1);
	if (tmp == NULL) return NULL;
	strcpy(tmp, newkey);
	return tmp;
}

static void *ValueCopy(void *prev, const void *nval)
{
	struct score *tmp = realloc(prev, sizeof(*tmp));
	if (tmp == NULL) return NULL;
	*(struct score *)tmp = *(struct score*)nval;
	return tmp;
}

static void ShowPoint(const void *key, void *val, va_list ap)
{
	FILE *fp = va_arg(ap, FILE *);
	const char *chan = va_arg(ap, const char *);
	if (key && val)
		fprintf(fp, "NOTICE %s :%11s = %16lld / %3llu\r\n", chan, 
				(char *)key, ((struct score *)val)->score,
				((struct score *)val)->dice);
	return;
}

int main(int argc, char *argv[])
{
	int sock;
	FILE *fwp, *frp, *pp;
	char buf[2048];
	mknmap mp;

	if (!(mp = NewMap((void *)strcmp, KeyCopy, ValueCopy, free, free))) {
		perror("NewMap");
		exit(EXIT_FAILURE);
	}

	sock = open_connection(argv[1], "6667");
	if (!(fwp = fdopen(sock, "w"))) {
		perror("fdopen(fwp)");
		exit(EXIT_FAILURE);
	}
	if (!(frp = fdopen(sock, "r"))) {
		perror("fdopen(frp)");
		exit(EXIT_FAILURE);
	}
	setbuf(fwp, NULL);
	setbuf(frp, NULL);

	fprintf(fwp, "NICK NkoDice\r\n");
	fprintf(fwp, "USER NkoDice NkoDice NkoDice NkoDice\r\n");
	fprintf(fwp, "JOIN %s\r\n", argv[2]);
	while (fgets(buf, sizeof(buf), frp)) {
		if (strstr(buf, "$START$")) {
			RemoveAll(mp);
		}
		if (strstr(buf, "$END$")) {
			ForEach(mp, ShowPoint, fwp, argv[2]);
		}
		if (strstr(buf, "$$$DICE50$$$")) {
			char cmd[256] = { 0 };
			*(strchr(buf, '!')) = 0;
			sprintf(cmd,
				"for i in `seq 1 50`; do node nkodice.js '%s' 1; done", buf + 1);
			if (!(pp = popen(cmd, "r"))) {
				fprintf(fwp, "NOTICE %s InternalError\r\n",
						argv[2]);
				exit(EXIT_FAILURE);
			}
			while (fgets(buf, sizeof(buf), pp)) {
				// 結果発表を検出
				if (strstr(buf, "RESULT") == buf) {
					char Name[16] = { 0 };
					long long pt = 0;
					struct score *tmp, sc = { 0, 0 };
					sscanf(buf, "%*s%s%*s%lld%*s",
							Name, &pt);
					if (!!(tmp = GetItem(mp, Name)))
						sc = *tmp;
					sc.dice++;
					sc.score += pt;
					PutItem(mp, Name, &sc);
				}
				*(strchr(buf, '\n')) = 0;
				fprintf(fwp, "NOTICE %s :%s\r\n", argv[2], buf);
			}
			pclose(pp);
		}
		if (strstr(buf, "$DICE$")) {
			char cmd[256] = { 0 };
			*(strchr(buf, '!')) = 0;
			sprintf(cmd, "node nkodice.js '%s'", buf + 1);
			if (!(pp = popen(cmd, "r"))) {
				fprintf(fwp, "NOTICE %s InternalError\r\n",
						argv[2]);
				exit(EXIT_FAILURE);
			}
			while (fgets(buf, sizeof(buf), pp)) {
				// 結果発表を検出
				if (strstr(buf, "RESULT") == buf) {
					char Name[16] = { 0 };
					long long pt = 0;
					struct score *tmp, sc = { 0, 0 };
					sscanf(buf, "%*s%s%*s%lld%*s",
							Name, &pt);
					if (!!(tmp = GetItem(mp, Name)))
						sc = *tmp;
					sc.dice++;
					sc.score += pt;
					PutItem(mp, Name, &sc);
				}
				*(strchr(buf, '\n')) = 0;
				fprintf(fwp, "NOTICE %s :%s\r\n", argv[2], buf);
			}
			pclose(pp);
		}
		if (strstr(buf, "PING")) {
			fprintf(fwp, "PONG irc.karashi\r\n");
		}
	}

	/* NOT REACHED */
	exit(EXIT_FAILURE);
}

static int open_connection(const char *host, const char *service)
{
	int sock;
	struct addrinfo hints, *res, *ai;
	int err;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if ((err = getaddrinfo(host, service, &hints, &res)) != 0) {
		fprintf(stderr, "getaddrinfo(3): %s\n", gai_strerror(err));
		exit(1);
	}
	for (ai = res; ai; ai = ai->ai_next) {
		sock = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
		if (sock < 0) {
			continue;
		}
		if (connect(sock, ai->ai_addr, ai->ai_addrlen) < 0) {
			close(sock);
			continue;
		}
		/* success */
		freeaddrinfo(res);
		return sock;
	}
	fprintf(stderr, "socket(2)/connect(2) failed");
	freeaddrinfo(res);
	exit(1);
}
