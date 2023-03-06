nginx 모듈에 대한 구조를 파악하기 위한 샘플 코드입니다 ' ㅇ')/
단순히 들어온 요청에 대해 plain/text로 Hello, world!를 출력해요.
추가적으로, error.log에도 마찬가지로 Hello, world!를 남기게 됩니다.

```
location /hello {
  hello_world "Hello, world!";
}
```
nginx.conf 파일에 위와 같이 작성할 경우, /hello로 접근하면 hello_world 모듈이 동작해요.

## commands 구조체에 대해
hello_world는 아래와 같이 `ngx_http_<module name>_commands` 구조체에서 설정할 수 있는데, ngx_string()으로 command의 이름과 각 플래그를 설정할 수 있답니다.  
```
static ngx_command_t ngx_http_hello_world_commands[] = {
  { 
    ngx_string("hello_world"),
    NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
    ngx_conf_set_str_slot,
    NGX_HTTP_LOC_CONF_OFFSET,
    offsetof(ngx_http_hello_world_loc_conf_t, message),
    NULL
  },

  ngx_null_command
};
```
그 뒤에 따라오는 구문인 `NGX_HTTP_LOC_CONF`는 'nginx 설정 파일의 location에서 사용 가능'을 의미하고, `NGX_CONF_TAKE1`는 하나의 인자값을 가진다는 걸 의미해요. 그래서 `offsetof(ngx_http_hello_world_loc_conf_t, message)`라고 되어있는 부분을 보면, 이미 정의해놓은 구조체 `ngx_http_hello_world_loc_conf_t`에 정의되어있는 `message`에 주어진 인자값을 저장한다는 걸 알 수 있죠. 자세한 command 구조체 설정에 대해서는 [NGINX 개발 가이드 - 새로운 모듈 추가](https://nginx.org/en/docs/dev/development_guide.html#adding_new_modules) 페이지를 참조하면 된답니다.

## handler 함수에 대해
실제 HTTP 요청에 대한 처리는 아래와 같이 `ngx_http_<module name>_handler()`함수에서 처리된답니다. 지금은 인자값으로 주어진 message를 사용해서 응답 및 로그를 남기는 게 아니라, 하드코딩 된 "Hello, world!"문자열을 출력하고 있어요. 아무튼 응답을 어떻게 하는지부터 봅시다.
```
static ngx_int_t ngx_http_hello_world_handler(ngx_http_request_t *r)
{
    ngx_buf_t *b;
    ngx_chain_t out;
    u_char *my_response = (u_char *) "Hello, world!";
    size_t my_response_length = sizeof("Hello, world!") - 1;
    ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, my_response);

    if (!(r->method & (NGX_HTTP_GET|NGX_HTTP_HEAD))) {
        return NGX_HTTP_NOT_ALLOWED;
    }

    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_length_n = my_response_length;
    r->headers_out.content_type.len = sizeof("text/plain") - 1;
    r->headers_out.content_type.data = (u_char *) "text/plain";

    if (r->method == NGX_HTTP_HEAD) {
        return ngx_http_send_header(r);
    }

    b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));
    if (b == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    out.buf = b;
    out.next = NULL;

    b->pos = my_response;
    b->last = my_response + my_response_length;
    b->memory = 1;
    b->last_buf = 1;

    ngx_int_t rc = ngx_http_send_header(r);
    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
        return rc;
    }

    return ngx_http_output_filter(r, &out);
}

```

먼저 헤더에 응답 코드를 NGX_HTTP_OK(200)으로 설정하고, `content-type`을 `text/plain`으로 설정해주는 걸 볼 수 있어요. 
```
	r->headers_out.status = NGX_HTTP_OK;
	r->headers_out.content_length_n = my_response_length;
	r->headers_out.content_type.len = sizeof("text/plain") - 1;
	r->headers_out.content_type.data = (u_char *) "text/plain";
```

그리고 아래와 같이 `ngx_http_send_header()`를 호출해서, 헤더를 먼저 전송합니다.
```
	ngx_int_t rc = ngx_http_send_header(r);
```

이미 헤더에 `content-type`을 `text/plain`으로 설정했으니, 이제 문자열만 바디에 써주면 간단하게 응답으로 "Hello, world!"를 보낼 수 있어요.
```
	ngx_buf_t *b;
	ngx_chain_t out;
	u_char *my_response = (u_char *) "Hello, world!";
	size_t my_response_length = sizeof("Hello, world!") - 1;

	...

	out.buf = b;
	out.next = NULL;

	b->pos = my_response;
	b->last = my_response + my_response_length;
	b->memory = 1;
	b->last_buf = 1;

	...

	return ngx_http_output_filter(r, &out);
```
우선 `ngx_buf_t`를 사용해서 버퍼 포인터를 만들고, `ngx_chain_t` 구조체 `out`에 할당해줍니다. 처리해줄 내용은 "Hello, world!" 문자열 하나이므로, `out.next`는 `NULL`로 할당해줍니다. ChatGPT한테 요청한 샘플 코드라서 그런가... 초기화 구문이 없네요... 아무튼. 

마지막으로 설정이 끝난 `ngx_chain_t` 구조체 `out`의 주소값을 `ngx_http_output_filter`에 인자값으로 전달하기만 하면 끗! ' ㅇ')
