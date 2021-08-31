typedef BOOL(WINAPI* WGLSWAPBUFFERSPROC)(HDC);
typedef void(WINAPI* GLCOLOR4FPROC)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);

bool HookOpenGL();
void UnHookOpenGL();