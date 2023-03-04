#include <iostream>
#include "uv.h"
#include "libuv/test/task.h"

static uv_loop_t* loop;

void echo_alloc(uv_handle_t* handle,
	size_t suggested_size,
	uv_buf_t* buf)
{
	buf->base = static_cast<char*>(malloc(suggested_size));
	buf->len = suggested_size;
}

typedef struct
{
	uv_write_t req;
	uv_buf_t buf;
} write_req_t;

void on_close(uv_handle_t* peer)
{
	free(peer);
}

void after_shutdown(uv_shutdown_t* req, int status)
{
	uv_close((uv_handle_t*)req->handle, on_close);
	free(req);
}

void after_write(uv_write_t* req, int status)
{
	write_req_t* wr;

	/* Free the read/write buffer and the request */
	wr = (write_req_t*)req;
	free(wr->buf.base);
	free(wr);

	if (status == 0)
		return;

	fprintf(stderr,
		"uv_write error: %s - %s\n",
		uv_err_name(status),
		uv_strerror(status));
}

void on_shutdown(uv_shutdown_t* req, int status)
{
	ASSERT_EQ(status, 0);
	free(req);
}

static bool server_closed = false;
static uv_handle_t* server;

void on_server_close(uv_handle_t* handle)
{
	ASSERT(handle == server);
}

std::string getTimeStringOfNow()
{
	struct tm fuckMSVC{};
	time_t now;
	time(&now);
	localtime_s(&fuckMSVC, &now);
	char text[100];
	strftime(text, sizeof text, "%F %T", &fuckMSVC);
	return { text };
}

void after_read(uv_stream_t* handle,
	ssize_t nread,
	const uv_buf_t* buf)
{
	int i;
	write_req_t* wr;
	uv_shutdown_t* sreq;
	int shutdown = 0;

	/* peer closes connection */
	if (nread < 0)
	{
		free(buf->base);
		sreq = static_cast<uv_shutdown_t*>(malloc(sizeof *sreq));
		if (uv_is_writable(handle))
		{
			uv_shutdown(sreq, handle, after_shutdown);
		}
		return;
	}

	if (nread == 0)
	{
		/* Everything OK, but nothing read. */
		free(buf->base);
		return;
	}

	for (i = 0; i < nread; i++)
	{
		if (buf->base[i] == 'Q')
		{
			if (i + 1 < nread && buf->base[i + 1] == 'S')
			{
				int reset = 0;
				if (i + 2 < nread && buf->base[i + 2] == 'S')
					shutdown = 1;
				if (i + 2 < nread && buf->base[i + 2] == 'H')
					reset = 1;
				if (reset && handle->type == UV_TCP)
					ASSERT_EQ(0, uv_tcp_close_reset((uv_tcp_t*)handle, on_close));
				else if (shutdown)
					break;
				else
					uv_close((uv_handle_t*)handle, on_close);
				free(buf->base);
				return;
			}
			else if (!server_closed)
			{
				uv_close(server, on_server_close);
				server_closed = true;
			}
		}
	}

	if (shutdown)
		ASSERT_EQ(0, uv_shutdown(
			static_cast<uv_shutdown_t*>(malloc(sizeof *sreq)),
			handle,
			on_shutdown
		));
}

void on_connection(uv_stream_t* srv, int status)
{
	if (status != 0)
	{
		fprintf(stderr, "Connect error %s\n", uv_err_name(status));
	}

	auto stream = (uv_stream_t*)(malloc(sizeof(uv_tcp_t)));
	uv_tcp_init(loop, (uv_tcp_t*)stream);

	/* associate srv with stream */
	stream->data = srv;

	uv_accept(srv, stream);
	uv_read_start(stream, echo_alloc, after_read);
}

int main()
{
	uv_tcp_t tcpServer;
	server = reinterpret_cast<uv_handle_t*>(&tcpServer);
	printf("Listening on 0.0.0.0:22222");

	struct sockaddr_in addr{};
	ASSERT_EQ(0, uv_ip4_addr("0.0.0.0", 22222, &addr));

	loop = uv_default_loop();
	int r;
	r = uv_tcp_init(loop, &tcpServer);
	if (r)
	{
		/* TODO: Error codes */
		fprintf(stderr, "Socket creation error\n");
		return 1;
	}

	r = uv_tcp_bind(&tcpServer, (const struct sockaddr*)&addr, 0);
	if (r)
	{
		/* TODO: Error codes */
		fprintf(stderr, "Bind error\n");
		return 1;
	}

	r = uv_listen((uv_stream_t*)&tcpServer, SOMAXCONN, on_connection);
	if (r)
	{
		/* TODO: Error codes */
		fprintf(stderr, "Listen error %s\n", uv_err_name(r));
		return 1;
	}

	notify_parent_process();
	uv_run(loop, UV_RUN_DEFAULT);
	return 0;
}
