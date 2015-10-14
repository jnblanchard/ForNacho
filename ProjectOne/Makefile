all: MCP

MCP: mcp.o
	g++ mcp.o -o MCP

mcp.o: mcp.cpp
	g++ -c mcp.cpp

clean:
	rm -rf *o MCP