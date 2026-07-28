/**
 * @file    filter.h
 * @brief   常用数字滤波器库，支持滑动平均、中值、低通、卡尔曼等
 * @note    所有滤波器以结构体封装状态，参数可传递且提供默认值，
 *          不依赖任何硬件外设，可在任意位置调用。
 */

#ifndef __FILTER_H__
#define __FILTER_H__

#include <stdint.h>
#include <stddef.h>   /* for size_t */

#ifdef __cplusplus
extern "C" {
#endif

/*==================================================================
 *  默认参数宏定义（可在调用 Init 前通过传参覆盖）
 *==================================================================*/

/* ---- 限幅滤波 ---- */
#define FILTER_LIMIT_DEFAULT_MAX_STEP    10.0f   /**< 默认最大允许变化量 */

/* ---- 滑动平均滤波 ---- */
#define FILTER_MA_DEFAULT_WINDOW         8       /**< 默认滑动窗口大小 */

/* ---- 加权滑动平均 ---- */
#define FILTER_WMA_DEFAULT_WINDOW        8       /**< 默认加权窗口大小 */

/* ---- 一阶低通滤波 ---- */
#define FILTER_LPF_DEFAULT_ALPHA         0.1f    /**< 默认滤波系数 (0~1), 越小越平滑 */

/* ---- 中值滤波 ---- */
#define FILTER_MEDIAN_DEFAULT_WINDOW     5       /**< 默认中值滤波窗口 */

/* ---- 卡尔曼滤波 ---- */
#define FILTER_KALMAN_DEFAULT_Q          0.01f   /**< 默认过程噪声协方差 */
#define FILTER_KALMAN_DEFAULT_R          0.1f    /**< 默认测量噪声协方差 */

/*==================================================================
 *  滤波器结构体定义
 *==================================================================*/

/**
 * @brief  限幅滤波器
 * @note   若本次值与上次值之差超过 max_step，则只允许变化 max_step
 */
typedef struct {
    float last_value;       /**< 上一次输出值 */
    float max_step;         /**< 最大允许步进变化量 */
    uint8_t is_first;       /**< 是否为首次调用 */
} Filter_Limit_TypeDef;

/**
 * @brief  滑动平均滤波器
 * @note   维护一个固定窗口的历史值，输出窗口内所有值的算术平均
 */
typedef struct {
    float *buffer;          /**< 历史数据缓冲区 (需外部分配) */
    float sum;              /**< 当前窗口内数据之和 */
    size_t window_size;     /**< 窗口大小 */
    size_t index;           /**< 当前写入位置 */
    size_t count;           /**< 已填充的数据个数 */
    uint8_t buffer_owned;   /**< 是否由滤波器内部管理缓冲区 */
} Filter_MA_TypeDef;

/**
 * @brief  加权滑动平均滤波器
 * @note   窗口内越新的数据权重越大
 */
typedef struct {
    float *buffer;          /**< 历史数据缓冲区 (需外部分配) */
    float *weights;         /**< 权重数组 (需外部分配) */
    size_t window_size;     /**< 窗口大小 */
    size_t index;           /**< 当前写入位置 */
    size_t count;           /**< 已填充的数据个数 */
    uint8_t buffer_owned;   /**< 是否由滤波器内部管理缓冲区 */
    uint8_t weights_owned;  /**< 是否由滤波器内部管理权重数组 */
} Filter_WMA_TypeDef;

/**
 * @brief  一阶低通滤波器 (IIR)
 * @note   公式: y[n] = alpha * x[n] + (1 - alpha) * y[n-1]
 *         alpha 越小滤波越强，alpha=1 则不过滤
 */
typedef struct {
    float alpha;            /**< 滤波系数 (0~1) */
    float last_output;      /**< 上一次输出值 */
    uint8_t is_first;       /**< 是否为首次调用 */
} Filter_LPF_TypeDef;

/**
 * @brief  中值滤波器
 * @note   对窗口内数据排序后取中间值，对脉冲噪声抑制效果好
 */
typedef struct {
    float *buffer;          /**< 历史数据缓冲区 (需外部分配) */
    size_t window_size;     /**< 窗口大小 (建议奇数) */
    size_t index;           /**< 当前写入位置 */
    size_t count;           /**< 已填充的数据个数 */
    uint8_t buffer_owned;   /**< 是否由滤波器内部管理缓冲区 */
} Filter_Median_TypeDef;

/**
 * @brief  卡尔曼滤波器 (单维)
 * @note   适用于单个传感器信号的最优估计，如温度、距离、角度等
 */
typedef struct {
    float x;                /**< 状态估计值 (后验) */
    float p;                /**< 估计误差协方差 (后验) */
    float q;                /**< 过程噪声协方差 */
    float r;                /**< 测量噪声协方差 */
    float k;                /**< 卡尔曼增益 */
    uint8_t is_first;       /**< 是否为首次调用 */
} Filter_Kalman_TypeDef;

/*==================================================================
 *  限幅滤波 API
 *==================================================================*/

void Filter_Limit_Init(Filter_Limit_TypeDef *f, float max_step);
float Filter_Limit_Compute(Filter_Limit_TypeDef *f, float input);

/*==================================================================
 *  滑动平均滤波 API
 *==================================================================*/

/**
 * @brief 初始化滑动平均滤波器
 * @param f           滤波器实例指针
 * @param window_size 窗口大小 (传 0 则使用默认值 FILTER_MA_DEFAULT_WINDOW)
 * @param user_buffer 用户提供的缓冲区 (传 NULL 则内部自动分配)
 */
void Filter_MA_Init(Filter_MA_TypeDef *f, size_t window_size, float *user_buffer);
void Filter_MA_DeInit(Filter_MA_TypeDef *f);
float Filter_MA_Compute(Filter_MA_TypeDef *f, float input);
void Filter_MA_Reset(Filter_MA_TypeDef *f);

/*==================================================================
 *  加权滑动平均滤波 API
 *==================================================================*/

void Filter_WMA_Init(Filter_WMA_TypeDef *f, size_t window_size,
                     float *user_buffer, float *user_weights);
void Filter_WMA_DeInit(Filter_WMA_TypeDef *f);
float Filter_WMA_Compute(Filter_WMA_TypeDef *f, float input);
void Filter_WMA_Reset(Filter_WMA_TypeDef *f);

/*==================================================================
 *  一阶低通滤波 API
 *==================================================================*/

/**
 * @brief 初始化一阶低通滤波器
 * @param f     滤波器实例指针
 * @param alpha 滤波系数 (0~1)，传 0 则使用默认值 FILTER_LPF_DEFAULT_ALPHA
 */
void Filter_LPF_Init(Filter_LPF_TypeDef *f, float alpha);
float Filter_LPF_Compute(Filter_LPF_TypeDef *f, float input);
void Filter_LPF_Reset(Filter_LPF_TypeDef *f);
void Filter_LPF_SetAlpha(Filter_LPF_TypeDef *f, float alpha);

/*==================================================================
 *  中值滤波 API
 *==================================================================*/

void Filter_Median_Init(Filter_Median_TypeDef *f, size_t window_size,
                        float *user_buffer);
void Filter_Median_DeInit(Filter_Median_TypeDef *f);
float Filter_Median_Compute(Filter_Median_TypeDef *f, float input);
void Filter_Median_Reset(Filter_Median_TypeDef *f);

/*==================================================================
 *  卡尔曼滤波 API
 *==================================================================*/

/**
 * @brief 初始化卡尔曼滤波器
 * @param f 滤波器实例指针
 * @param q 过程噪声协方差 (传 0 则使用默认值)
 * @param r 测量噪声协方差 (传 0 则使用默认值)
 * @param initial_value 初始状态估计值
 */
void Filter_Kalman_Init(Filter_Kalman_TypeDef *f, float q, float r,
                        float initial_value);
float Filter_Kalman_Compute(Filter_Kalman_TypeDef *f, float measurement);
void Filter_Kalman_Reset(Filter_Kalman_TypeDef *f, float initial_value);
void Filter_Kalman_SetQR(Filter_Kalman_TypeDef *f, float q, float r);

#ifdef __cplusplus
}
#endif

#endif /* __FILTER_H__ */
