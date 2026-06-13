SdlRenderer g_app;

int main() {
    if (g_app.Initialize())
	{
		g_app.Run();
	}
    g_app.Shutdown();
    return 0;
}
