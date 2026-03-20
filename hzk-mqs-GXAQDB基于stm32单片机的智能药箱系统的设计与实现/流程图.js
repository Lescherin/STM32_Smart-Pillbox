graph TD
%% 上电后进行各个硬件初始化
    A["硬件系统上电"] --> B["核心硬件初始化"]
    

    %% 硬件初始化流程（STM32主控）
    subgraph "硬件初始化（STM32主控）"
        B --> C["STM32F103C6T6 主控初始化"]
        %% 主控初始化 各自分支:传感器硬件、执行器硬件、通信硬件初始化
        
        %% 分支D传感器硬件初始化  
        C --> D["传感器硬件初始化"]
            D --> D1["DHT11（温湿度）引脚初始化(PB12)"]
            D --> D2["HX711（称重）引脚初始化(PB10/PB11)"]
            D --> D3["DS1302（时钟）引脚初始化(PB5/PB6/PB7)"]
            D --> D4["TCRT5000（红外）ADC通道初始化(PA1)"]
            D --> D5["KEY按钮（输入）引脚初始化(PAx/PAy)"]
        

        %% 分支E执行器硬件初始化
        C --> E["执行器硬件初始化"]
            E --> E1["SG90舵机(PB0) PWM初始化"]
            E --> E2["蜂鸣器(PA7) GPIO初始化"]
        
        %% 分支F通信硬件初始化
        C --> F["通信硬件初始化"]
            F --> F1["ESP8266 WiFi模块(PA2/PA3) 串口初始化"]
            F --> F2["OLED(I2C) 引脚初始化(PB8/PB9)"]
    end
    
    %% 传感器初始化完成后，直接开始数据采集
    D1 --> G1["DHT11通过1-Wire总线传输温湿度数据到STM32"]
    D2 --> G2["HX711通过2线串行接口传输重量数据到STM32"]
    D3 --> G3["DS1302通过3线串行接口传输时间数据到STM32"]
    D4 --> G4["TCRT5000通过ADC模拟信号传输门状态数据到STM32"]
    D5 --> G5["KEY按钮通过GPIO中断/轮询检测用户输入"]

    %% 采集数据后进行数据处理（ADC转换/协议解析）
    G1 & G2 & G3 & G4 & G5 --> L["STM32硬件层面数据处理（ADC转换/协议解析）"]
    
    %% 线上数据传输流程（硬件链路）
    subgraph "线上数据传输（硬件链路）"
        %% ESP8266初始化完成后，等待STM32发送数据
        L --> M["STM32通过串口将数据发送到ESP8266 WiFi模块"]
        M --> N["ESP8266硬件联网，通过MQTT协议线上上传数据到云端"]
        N --> O["云端接收并存储硬件上传的传感器数据"]

        %% 同时等待OLED初始化完成后，等待STM32发送显示数据
        L --> U["OLED硬件显示：实时展示传感器数据（本地硬件输出）"]
    end
    



    %% 按键本地控制流程
    subgraph "按键本地控制流程"
        G5 --> K["检测到KEY按钮触发事件"]
        K --> K1{按键类型判断}
        K1 -->|紧急关闭| K2["SG90舵机关闭药箱门"]
        K1 -->|确认服药| K3["停止蜂鸣器/语音提醒"]
        K1 -->|手动出药| K4["控制SG90舵机打开药箱门"]
        K2 & K3 & K4 --> K5["更新OLED显示按键操作状态"]
    end
    
    %%云端指令硬件执行流程
    subgraph "云端指令硬件执行流程"
        O --> P["云端下发控制指令，ESP8266线上接收"]
        P --> Q["ESP8266通过串口将指令转发到STM32"]
        Q --> R["STM32驱动硬件执行器"]
        %% 舵机和蜂鸣器初始化完成后，等待STM32的驱动指令
            R --> S["SG90舵机（机械动作）：控制药箱开关"]
            R --> T["蜂鸣器（声光报警）：异常温湿度/门状态提醒"]
    end
    

    
    style A fill:#f9f,stroke:#333,stroke-width:2px
    style B fill:#ff9,stroke:#333,stroke-width:2px
    style C fill:#7df,stroke:#333,stroke-width:2px
    style D fill:#7dd,stroke:#333,stroke-width:2px
    style E fill:#7db,stroke:#333,stroke-width:2px
    style F fill:#8da,stroke:#333,stroke-width:2px
    style L fill:#f90,stroke:#333,stroke-width:2px
    style U fill:#f99,stroke:#333,stroke-width:2px
    style M fill:#09f,stroke:#333,stroke-width:2px
    style N fill:#09a,stroke:#333,stroke-width:2px
    style O fill:#09a,stroke:#333,stroke-width:2px
    style P fill:#99D,stroke:#333,stroke-width:2px
    style Q fill:#99D,stroke:#333,stroke-width:2px
    style R fill:#8C8,stroke:#333,stroke-width:2px
    style D5 fill:#7dd,stroke:#333,stroke-width:2px
    style G5 fill:#9f9,stroke:#333,stroke-width:2px
    style K fill:#f9f,stroke:#333,stroke-width:2px
    style K1 fill:#ff9,stroke:#333,stroke-width:2px
    style K2 fill:#7df,stroke:#333,stroke-width:2px
    style K3 fill:#7df,stroke:#333,stroke-width:2px
    style K4 fill:#7df,stroke:#333,stroke-width:2px
    style K5 fill:#9f9,stroke:#333,stroke-width:2px