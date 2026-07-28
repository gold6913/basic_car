# 🔧 Filter 数字滤波器库使用手册

> **版本**: v1.0  
> **适用平台**: MSPM0G3507 / Cortex-M0+（亦可移植到任意 C 平台）  
> **依赖**: 仅依赖标准库 `<stdlib.h>`、`<string.h>`，与硬件完全解耦

---

## 目录

- [1. 快速入门](#1-快速入门)
- [2. 滤波器选型指南](#2-滤波器选型指南)
- [3. API 参考](#3-api-参考)
  - [3.1 限幅滤波 (Limit)](#31-限幅滤波-limit)
  - [3.2 滑动平均滤波 (MA)](#32-滑动平均滤波-ma)
  - [3.3 加权滑动平均滤波 (WMA)](#33-加权滑动平均滤波-wma)
  - [3.4 一阶低通滤波 (LPF)](#34-一阶低通滤波-lpf)
  - [3.5 中值滤波 (Median)](#35-中值滤波-median)
  - [3.6 卡尔曼滤波 (Kalman)](#36-卡尔曼滤波-kalman)
- [4. 缓冲区策略](#4-缓冲区策略)
- [5. 默认参数表](#5-默认参数表)
- [6. 常见场景示例](#6-常见场景示例)
- [7. 性能与内存](#7-性能与内存)

---

## 1. 快速入门

```c
#include "filter.h"

void demo(void)
{
    /* ---- 方式 A：使用默认参数（推荐新手） ---- */
    Filter_LPF_TypeDef lpf;
    Filter_LPF_Init(&lpf, 0);                  // alpha 自动取默认 0.1

    /* ---- 方式 B：自定义参数 ---- */
    Filter_LPF_TypeDef lpf2;
    Filter_LPF_Init(&lpf2, 0.05f);             // 更平滑

    /* ---- 循环中调用 ---- */
    while (1)
    {
        float raw = read_sensor();
        float smooth = Filter_LPF_Compute(&lpf, raw);
        // 使用 smooth 替代 raw
    }
}
```

---

## 2. 滤波器选型指南

| 场景 | 推荐滤波器 | 理由 |
|------|-----------|------|
| ADC 采样有毛刺/跳变 | **限幅滤波** | 抑制突变干扰，实现简单 |
| ADC 周期性噪声（工频 50Hz） | **滑动平均** | 对周期性干扰抑制极好 |
| 需要快速响应 + 平滑 | **加权滑动平均** | 新数据权重大，兼顾响应速度 |
| 通用平滑，计算资源极少 | **一阶低通 (LPF)** | 仅需 3 次浮点运算 |
| 传感器偶发坏点（脉冲噪声） | **中值滤波** | 对孤立野点完全免疫 |
| 陀螺仪/加速度计融合 | **卡尔曼** | 融合模型预测与测量值，最优估计 |
| 高噪声 + 需保留突变 | **限幅 + 低通 串联** | 先剔除突变，再平滑 |

---

## 3. API 参考

### 3.1 限幅滤波 (Limit)

> **原理**: 若本次采样值与上次输出之差超过 `max_step`，则只允许变化 `max_step`。  
> **适用**: GPIO 按键消抖、传感器跳变保护。

```c
Filter_Limit_TypeDef f;

// 初始化：max_step=0 则使用默认 10.0
Filter_Limit_Init(&f, 0);

// 计算
float out = Filter_Limit_Compute(&f, input);
```

| 参数 | 含义 | 默认值 |
|------|------|--------|
| `max_step` | 最大允许步进 | `10.0f` |

> **调参建议**: 根据传感器正常变化速率设定。ADC 值跳变通常设为信号范围的 5%~10%。

---

### 3.2 滑动平均滤波 (MA)

> **原理**: 维护长度为 N 的 FIFO 队列，输出队内所有值的算术平均。  
> **适用**: 周期性噪声（如 50Hz 工频干扰，采样率设为工频的整数倍时效果最佳）。

```c
Filter_MA_TypeDef f;

// window_size=0 → 默认 8; user_buffer=NULL → 自动分配
Filter_MA_Init(&f, 0, NULL);

float out = Filter_MA_Compute(&f, input);

// 使用完毕后释放（仅当使用了自动分配时）
Filter_MA_DeInit(&f);
```

| API | 说明 |
|-----|------|
| `Filter_MA_Init(f, size, buf)` | 初始化，`size=0` → 默认 8 |
| `Filter_MA_Compute(f, input)` | 输入新值，返回滤波结果 |
| `Filter_MA_Reset(f)` | 清空历史，从头开始 |
| `Filter_MA_DeInit(f)` | 释放内部缓冲区 |

> **调参建议**: 窗口越大越平滑，但响应越慢。ADC 常用 4~16。

---

### 3.3 加权滑动平均滤波 (WMA)

> **原理**: 与 MA 类似，但窗口内越新的数据权重越大（默认权重线性递增）。  
> **适用**: 需要兼顾平滑度和响应速度的场景。

```c
Filter_WMA_TypeDef f;

Filter_WMA_Init(&f, 0, NULL, NULL);   // 全部默认
float out = Filter_WMA_Compute(&f, input);
Filter_WMA_DeInit(&f);
```

| API | 说明 |
|-----|------|
| `Filter_WMA_Init(f, size, buf, w)` | `w=NULL` → 自动生成线性递增权重并归一化 |
| `Filter_WMA_Compute(f, input)` | 加权平均计算 |
| `Filter_WMA_Reset(f)` | 清空历史 |
| `Filter_WMA_DeInit(f)` | 释放内部缓冲区 |

---

### 3.4 一阶低通滤波 (LPF)

> **原理**: $y_n = \alpha \cdot x_n + (1-\alpha) \cdot y_{n-1}$  
> **适用**: 最通用的平滑滤波，计算量极小。

```c
Filter_LPF_TypeDef f;

Filter_LPF_Init(&f, 0);              // alpha 默认 0.1
float out = Filter_LPF_Compute(&f, input);

// 运行时动态调整滤波强度
Filter_LPF_SetAlpha(&f, 0.05f);      // 更强滤波
```

| API | 说明 |
|-----|------|
| `Filter_LPF_Init(f, alpha)` | `alpha=0` → 默认 0.1 |
| `Filter_LPF_Compute(f, input)` | 低通滤波计算 |
| `Filter_LPF_Reset(f)` | 重置内部状态 |
| `Filter_LPF_SetAlpha(f, alpha)` | 在线修改滤波系数 |

| $\alpha$ 值 | 截止频率（$f_s$=采样率） | 效果 |
|:-----------:|:--------------------------|:-----|
| 0.01 | $0.0016 \cdot f_s$ | 极强滤波，响应很慢 |
| 0.05 | $0.008 \cdot f_s$ | 强滤波 |
| **0.1** | $0.017 \cdot f_s$ | **默认，适中** |
| 0.3 | $0.057 \cdot f_s$ | 较轻滤波 |
| 0.5 | $0.11 \cdot f_s$ | 弱滤波，响应快 |
| 1.0 | 无滤波 | 输出 = 输入 |

---

### 3.5 中值滤波 (Median)

> **原理**: 对窗口内数据排序，取中间值。偶数窗口取中间两数均值。  
> **适用**: 脉冲噪声、传感器偶然坏点。对高斯噪声效果不如滑动平均。

```c
Filter_Median_TypeDef f;

Filter_Median_Init(&f, 0, NULL);      // 默认窗口 5（奇数）
float out = Filter_Median_Compute(&f, input);
Filter_Median_DeInit(&f);
```

| API | 说明 |
|-----|------|
| `Filter_Median_Init(f, size, buf)` | `size=0` → 默认 5 |
| `Filter_Median_Compute(f, input)` | 返回窗口内中值 |
| `Filter_Median_Reset(f)` | 清空历史 |
| `Filter_Median_DeInit(f)` | 释放内部缓冲区 |

> **注意**: 窗口建议取奇数（3/5/7），偶数窗口涉及均值运算，略有额外开销。

---

### 3.6 卡尔曼滤波 (Kalman)

> **原理**: 预测-更新两步迭代，融合过程模型与测量值，得到最小均方误差意义下的最优估计。  
> **适用**: 陀螺仪角度、加速度计倾角、TOF 测距等传感器的精确估计。

```c
Filter_Kalman_TypeDef f;

// q=0 → 默认 0.01; r=0 → 默认 0.1; 初值=0
Filter_Kalman_Init(&f, 0, 0, 0);

float est = Filter_Kalman_Compute(&f, measurement);

// 运行时调整噪声参数
Filter_Kalman_SetQR(&f, 0.001, 0.5);
```

| API | 说明 |
|-----|------|
| `Filter_Kalman_Init(f, q, r, init)` | `q=0/r=0` → 使用默认值 |
| `Filter_Kalman_Compute(f, meas)` | 输入测量值，返回估计值 |
| `Filter_Kalman_Reset(f, init)` | 重置状态 |
| `Filter_Kalman_SetQR(f, q, r)` | 在线修改 Q/R |

| 参数 | 含义 | 默认值 | 调参方向 |
|:----:|------|:------:|----------|
| **Q** | 过程噪声协方差 | `0.01` | Q↑ → 更信任测量值，响应更快但更噪声 |
| **R** | 测量噪声协方差 | `0.1` | R↑ → 更信任模型预测，输出更平滑但响应更慢 |

> **调参口诀**:  
> - 信号抖动大 → **增大 R**（告诉滤波器"传感器不可靠"）  
> - 信号变化快 → **增大 Q**（告诉滤波器"系统本身在快速变化"）  
> - 一般保持 $Q : R \approx 1 : 10$ 作为起点

---

## 4. 缓冲区策略

每个需要窗口的滤波器（MA / WMA / Median）都支持两种内存模式：

| 模式 | 用法 | 优点 | 缺点 |
|------|------|------|------|
| **自动分配** | `Init(&f, size, NULL)` | 省心，无需管理 | 需 heap，有碎片风险 |
| **外部静态** | `Init(&f, size, my_buf)` | 无 heap，确定性 | 需预先声明数组 |

```c
// 模式 1：自动分配（简单）
Filter_MA_Init(&f1, 8, NULL);
// ... 使用 ...
Filter_MA_DeInit(&f1);          // ⚠️ 记得释放！

// 模式 2：静态缓冲区（嵌入式推荐）
static float buf[8];
Filter_MA_Init(&f2, 8, buf);    // 不调用 DeInit 也没事
```

---

## 5. 默认参数表

| 宏定义 | 默认值 | 说明 |
|--------|:------:|------|
| `FILTER_LIMIT_DEFAULT_MAX_STEP` | `10.0` | 限幅最大步进 |
| `FILTER_MA_DEFAULT_WINDOW` | `8` | 滑动平均窗口 |
| `FILTER_WMA_DEFAULT_WINDOW` | `8` | 加权滑动平均窗口 |
| `FILTER_LPF_DEFAULT_ALPHA` | `0.1` | 低通滤波系数 |
| `FILTER_MEDIAN_DEFAULT_WINDOW` | `5` | 中值滤波窗口 |
| `FILTER_KALMAN_DEFAULT_Q` | `0.01` | 卡尔曼过程噪声 |
| `FILTER_KALMAN_DEFAULT_R` | `0.1` | 卡尔曼测量噪声 |

> 可通过修改 `filter.h` 中的宏定义来更改全局默认值。

---

## 6. 常见场景示例

### 6.1 ADC 采样 + 低通 + 限幅（两级串联）

```c
Filter_LPF_TypeDef   adc_lpf;
Filter_Limit_TypeDef adc_limit;

Filter_LPF_Init(&adc_lpf, 0.1f);
Filter_Limit_Init(&adc_limit, 50.0f);

float read_adc_smooth(void)
{
    float raw = (float)DL_ADC12_getMemResult(ADC0);  // 原始 ADC
    float lpf = Filter_LPF_Compute(&adc_lpf, raw);   // 先低通平滑
    return Filter_Limit_Compute(&adc_limit, lpf);    // 再限幅保护
}
```

### 6.2 灰度传感器巡线（多通道滑动平均）

```c
#define GRAY_CHANNELS 8
#define MA_WINDOW     6

static float ma_buf[GRAY_CHANNELS][MA_WINDOW];
Filter_MA_TypeDef gray_ma[GRAY_CHANNELS];

void gray_filter_init(void)
{
    for (int i = 0; i < GRAY_CHANNELS; i++)
    {
        Filter_MA_Init(&gray_ma[i], MA_WINDOW, ma_buf[i]);
    }
}

void gray_filter(unsigned short *raw, float *out)
{
    for (int i = 0; i < GRAY_CHANNELS; i++)
    {
        out[i] = Filter_MA_Compute(&gray_ma[i], (float)raw[i]);
    }
}
```

### 6.3 陀螺仪偏航角卡尔曼滤波

```c
Filter_Kalman_TypeDef yaw_filter;

// 传感器噪声较大，测量协方差 R 设大一些
Filter_Kalman_Init(&yaw_filter, 0.001, 0.5, 0);

float get_filtered_yaw(void)
{
    float raw_yaw = imu->yaw;                      // JY61P 原始偏航角
    return Filter_Kalman_Compute(&yaw_filter, raw_yaw);
}
```

### 6.4 按键消抖（限幅 + 低通）

```c
Filter_LPF_TypeDef   key_lpf;
Filter_Limit_TypeDef key_limit;

Filter_LPF_Init(&key_lpf, 0.3f);       // 按键响应要快，alpha 稍大
Filter_Limit_Init(&key_limit, 3.0f);   // 数字量跳变限幅

uint8_t key_read_debounce(void)
{
    float raw = (float)DL_GPIO_readPins(KEY_PORT, KEY_PIN);
    float lpf = Filter_LPF_Compute(&key_lpf, raw);
    float lim = Filter_Limit_Compute(&key_limit, lpf);
    return (lim > 0.5f) ? 1 : 0;
}
```

---

## 7. 性能与内存

| 滤波器 | 内存占用 (float) | 每次 Compute 计算量 | 是否需要 DeInit |
|--------|:----------------:|:-------------------|:---------------:|
| **Limit** | 3 | 1 次减法 + 2 次比较 | ❌ |
| **LPF** | 3 | 2 次乘法 + 1 次加法 | ❌ |
| **Kalman** | 5 | 4 次加减 + 3 次乘除 | ❌ |
| **MA** | `4 + N` | 1 加减 + 1 除法 | ✅（自动分配时） |
| **WMA** | `4 + 2N` | N 次乘加 | ✅（自动分配时） |
| **Median** | `4 + N` | 排序 O(N²) + 拷贝 | ✅（自动分配时） |

> - **Limit / LPF / Kalman**: 无动态内存，无需 DeInit，适合中断中调用  
> - **MA / WMA / Median**: 使用静态缓冲区（`user_buffer` 参数）时也无需 DeInit  
> - N = 窗口大小

---

> 📁 文件位置: `app/filter/filter.h` `app/filter/filter.c`
