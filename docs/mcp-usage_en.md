# MCP Protocol for IoT Control: Usage Guide

> This document explains how to control ESP32 devices using the MCP protocol. For detailed protocol flows, see [`mcp-protocol.md`](./mcp-protocol_en.md).

## Introduction

MCP (Model Context Protocol) is a modern protocol recommended for IoT control. It uses standard JSON-RPC 2.0 to discover and invoke "tools" between the backend and devices, enabling flexible device control.

## Typical Flow

1. After boot, the device connects to the backend using a base transport (WebSocket/MQTT).
2. The backend initializes the session via the MCP method `initialize`.
3. The backend calls `tools/list` to discover all tools (features) supported by the device and their parameters.
4. The backend calls `tools/call` to invoke a tool and control the device.

For detailed formats and interactions, see [`mcp-protocol_en.md`](./mcp-protocol_en.md).

## Registering Tools on the Device

The device registers callable tools via `McpServer::AddTool`. The common signature is:

```cpp
void AddTool(
    const std::string& name,           // Tool name; unique and hierarchical e.g. self.dog.forward
    const std::string& description,    // Natural-language description for LLM understanding
    const PropertyList& properties,    // Input parameters (optional): bool, int, string
    std::function<ReturnValue(const PropertyList&)> callback // Callback when invoked
);
```
- name: unique tool identifier, recommended style "module.feature".
- description: human-readable description for AI/users.
- properties: parameter list; types supported: bool/int/string; may specify range/defaults.
- callback: executes when invoked; returns bool/int/string value.

## Example Registration (ESP-Hi)

```cpp
void InitializeTools() {
    auto& mcp_server = McpServer::GetInstance();
    // Example 1: no parameters, move the robot forward
    mcp_server.AddTool("self.dog.forward", "Move robot forward", PropertyList(), [this](const PropertyList&) -> ReturnValue {
        servo_dog_ctrl_send(DOG_STATE_FORWARD, NULL);
        return true;
    });
    // Example 2: with parameters, set RGB color
    mcp_server.AddTool("self.light.set_rgb", "Set RGB color", PropertyList({
        Property("r", kPropertyTypeInteger, 0, 255),
        Property("g", kPropertyTypeInteger, 0, 255),
        Property("b", kPropertyTypeInteger, 0, 255)
    }), [this](const PropertyList& properties) -> ReturnValue {
        int r = properties["r"].value<int>();
        int g = properties["g"].value<int>();
        int b = properties["b"].value<int>();
        led_on_ = true;
        SetLedColor(r, g, b);
        return true;
    });
}
```

## Common JSON-RPC Examples

### 1. List tools
```json
{
  "jsonrpc": "2.0",
  "method": "tools/list",
  "params": { "cursor": "" },
  "id": 1
}
```

### 2. Move chassis forward
```json
{
  "jsonrpc": "2.0",
  "method": "tools/call",
  "params": {
    "name": "self.chassis.go_forward",
    "arguments": {}
  },
  "id": 2
}
```

### 3. Switch light mode
```json
{
  "jsonrpc": "2.0",
  "method": "tools/call",
  "params": {
    "name": "self.chassis.switch_light_mode",
    "arguments": { "light_mode": 3 }
  },
  "id": 3
}
```

### 4. Flip camera
```json
{
  "jsonrpc": "2.0",
  "method": "tools/call",
  "params": {
    "name": "self.camera.set_camera_flipped",
    "arguments": {}
  },
  "id": 4
}
```

## Notes
- Treat tool names, parameters, and return values as authoritative per the device's `AddTool` registrations.
- For new projects, MCP is the recommended unified protocol for IoT control.
- For advanced usage, see [`mcp-protocol_en.md`](./mcp-protocol_en.md).
