# my_lexin
Custom hardware implementation of XiaoZhi AI WebSocket client + hardware control (non-universal)
## 硬件MSP功能注册流程  
1. **设备端发送hello**  
```json
{
  "type": "hello",
  "version": 1,
  "features": {
    "mcp": true//通知服务器设备支持mcp
  },
  "transport": "websocket",
  "audio_params": {
    "format": "opus",
    "sample_rate": 16000,
    "channels": 1,
    "frame_duration": 60
  }
}
```

2. **服务器请求初始化MCP**  
```json
{
  "type": "mcp",
  "payload": {
    "jsonrpc": "2.0",
    "method": "initialize",
    "id": 1,
    "params": {
      "protocolVersion": "2024-11-05",
      "capabilities": {
        "vision": {
          "url": "http://api.xiaozhi.me/vision/explain",
          "token": "5a1595ab-df08-487c-b373-47e29cad9809"
        }
      },
      "clientInfo": {
        "name": "xiaozhi-mqtt-client",
        "version": "1.0.0"
      }
    }
  },
  "session_id": "5a1595ab"
}
```

3. **设备端返回初始化结果**  
```json
{
  "session_id": "5a1595ab",
  "type": "mcp",
  "payload": {
    "jsonrpc": "2.0",
    "id": 1,
    "result": {
      "protocolVersion": "2024-11-05",
      "capabilities": {
        "tools": {
          "list": true,//回复服务端设备支持工具列表
          "call": true //回复服务端设备支持工具调用
        }
      },
      "serverInfo": {
        "name": "wangwang",
        "version": "1.8.2"
      }
    }
  }
}
```

4. **服务端通知已收到初始化结果**  
```json
{
  "type": "mcp",
  "payload": {
    "jsonrpc": "2.0",
    "method": "notifications/initialized"
  },
  "session_id": "5a1595ab"
}
```

5. **服务端申请toollist**  
```json
{
  "type": "mcp",
  "payload": {
    "jsonrpc": "2.0",
    "method": "tools/list",
    "id": 2,
    "params": {}
  },
  "session_id": "e18f7652"
}
```

6. **设备端返回toollist**  
```json
{
  "session_id": "e18f7652",
  "type": "mcp",
  "payload": {
    "jsonrpc": "2.0",
    "id": 2,
    "result": {
      "tools": [
          //功能1 设置音量
        {
          "name": "set_volume",
          "description": "Set the volume of the audio speaker, volume is 50 initially.",
          "inputSchema": {
            "type": "object",
            "required": ["volume"],
            "properties": {
              "volume": {
                "type": "integer",
                "minimum": 0,
                "maximum": 100
              }
            }
          }
        },
          //功能2 控制灯开关
        {
          "name": "light_switch",
          "description": "Control the light state (on/off)",
          "inputSchema": {
            "type": "object",
            "required": ["state"],
            "properties": {
              "state": {
                "type": "boolean"
              }
            }
          }
        }
      ]
    }
  }
}
```

## 硬件MSP功能调用流程  
1. **语音表明使用tool的意图**  
> 例如：声音小点或者我听不清

2. **服务端调用tool**  
```json
{
  "type": "mcp",
  "payload": {
    "jsonrpc": "2.0",
    "method": "tools/call",
    "id": 3,
    "params": {
      "name": "set_volume",
      "arguments": {
        "volume": 30
      }
    }
  },
  "session_id": "e18f7652"
}
```

3. **服务端回复tool是否执行**  
```json
{
  "session_id": "e18f7652",
  "type": "mcp",
  "payload": {
    "jsonrpc": "2.0",
    "id": 3,
    "result": {
      "content": [
        {
          "type": "text",
          "text": "true"
        }
      ],
      "isError": false
    }
  }
}
```

