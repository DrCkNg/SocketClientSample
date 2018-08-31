# SocketClientSample
Socket Client Sample is a simple Socket Client application which send trigger commands to a dedicated Port of a Server. Command format are customisable in setting file, with CR(13) LF(10) as string terminator. Each client waits for its reply from the Server and increase Good counter if the reply data is not [EMPTY]/ERROR/Disconnected, Bad Counter will be increased otherwise.
