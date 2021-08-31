typedef BOOL(WINAPI* WGLSWAPBUFFERSPROC)(HDC);
typedef void(WINAPI* GLCOLOR4FPROC)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef void(WINAPI* GLVIEWPORTPROC)(GLint x, GLint y, GLsizei width, GLsizei height);
typedef BOOL(WINAPI* WGLSWAPLAYERBUFFERSPROC)(HDC);

bool HookOpenGL();
bool UnHookOpenGL();