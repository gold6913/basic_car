/**
 * @file    filter.c
 * @brief   常用数字滤波器实现
 */

#include "filter.h"
#include <stdlib.h>   /* malloc, free */
#include <string.h>   /* memcpy */

/*==================================================================
 *  内部辅助函数
 *==================================================================*/

/**
 * @brief  冒泡排序（用于中值滤波）
 * @note   数据量小，冒泡足够；若需优化可换为选择排序
 */
static void bubble_sort(float *arr, size_t n)
{
    for (size_t i = 0; i < n - 1; i++)
    {
        for (size_t j = 0; j < n - 1 - i; j++)
        {
            if (arr[j] > arr[j + 1])
            {
                float tmp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = tmp;
            }
        }
    }
}

/*==================================================================
 *  限幅滤波
 *==================================================================*/

/**
 * @brief  初始化限幅滤波器
 * @param  f        滤波器实例指针
 * @param  max_step 最大允许变化步进，传 0 则使用默认值
 */
void Filter_Limit_Init(Filter_Limit_TypeDef *f, float max_step)
{
    f->last_value = 0.0f;
    f->max_step   = (max_step > 0.0f) ? max_step : FILTER_LIMIT_DEFAULT_MAX_STEP;
    f->is_first   = 1;
}

/**
 * @brief  限幅滤波计算
 * @param  f     滤波器实例指针
 * @param  input 当前采样值
 * @return 限幅后的值
 */
float Filter_Limit_Compute(Filter_Limit_TypeDef *f, float input)
{
    if (f->is_first)
    {
        f->last_value = input;
        f->is_first   = 0;
        return input;
    }

    float diff = input - f->last_value;
    if (diff > f->max_step)
    {
        f->last_value += f->max_step;
    }
    else if (diff < -f->max_step)
    {
        f->last_value -= f->max_step;
    }
    else
    {
        f->last_value = input;
    }
    return f->last_value;
}

/*==================================================================
 *  滑动平均滤波
 *==================================================================*/

/**
 * @brief  初始化滑动平均滤波器
 * @param  f           滤波器实例指针
 * @param  window_size 窗口大小，传 0 则使用默认
 * @param  user_buffer 用户缓冲区，传 NULL 则内部 malloc
 */
void Filter_MA_Init(Filter_MA_TypeDef *f, size_t window_size, float *user_buffer)
{
    if (window_size == 0) { window_size = FILTER_MA_DEFAULT_WINDOW; }

    f->window_size = window_size;
    f->sum         = 0.0f;
    f->index       = 0;
    f->count       = 0;

    if (user_buffer != NULL)
    {
        f->buffer       = user_buffer;
        f->buffer_owned = 0;
    }
    else
    {
        f->buffer       = (float *)calloc(window_size, sizeof(float));
        f->buffer_owned = 1;
    }
}

/**
 * @brief  释放滑动平均滤波器内部缓冲区
 */
void Filter_MA_DeInit(Filter_MA_TypeDef *f)
{
    if (f->buffer_owned && f->buffer != NULL)
    {
        free(f->buffer);
        f->buffer = NULL;
    }
    f->buffer_owned = 0;
    f->count        = 0;
    f->sum          = 0.0f;
    f->index        = 0;
}

/**
 * @brief  滑动平均滤波计算
 * @param  f     滤波器实例指针
 * @param  input 当前采样值
 * @return 滤波后的值
 */
float Filter_MA_Compute(Filter_MA_TypeDef *f, float input)
{
    /* 减去将被覆盖的旧值 */
    if (f->count == f->window_size)
    {
        f->sum -= f->buffer[f->index];
    }

    /* 写入新值 */
    f->buffer[f->index] = input;
    f->sum += input;

    f->index++;
    if (f->index >= f->window_size)
    {
        f->index = 0;
    }

    if (f->count < f->window_size)
    {
        f->count++;
    }

    return f->sum / (float)f->count;
}

/**
 * @brief  重置滑动平均滤波器
 */
void Filter_MA_Reset(Filter_MA_TypeDef *f)
{
    for (size_t i = 0; i < f->window_size; i++)
    {
        f->buffer[i] = 0.0f;
    }
    f->sum   = 0.0f;
    f->index = 0;
    f->count = 0;
}

/*==================================================================
 *  加权滑动平均滤波
 *==================================================================*/

/**
 * @brief  初始化加权滑动平均滤波器
 * @param  f            滤波器实例指针
 * @param  window_size  窗口大小，传 0 则使用默认
 * @param  user_buffer  用户缓冲区，传 NULL 则内部 malloc
 * @param  user_weights 用户权重数组，传 NULL 则内部生成线性递增权重
 */
void Filter_WMA_Init(Filter_WMA_TypeDef *f, size_t window_size,
                     float *user_buffer, float *user_weights)
{
    if (window_size == 0) { window_size = FILTER_WMA_DEFAULT_WINDOW; }

    f->window_size = window_size;
    f->index       = 0;
    f->count       = 0;

    /* ---- 缓冲区 ---- */
    if (user_buffer != NULL)
    {
        f->buffer       = user_buffer;
        f->buffer_owned = 0;
    }
    else
    {
        f->buffer       = (float *)calloc(window_size, sizeof(float));
        f->buffer_owned = 1;
    }

    /* ---- 权重 ---- */
    if (user_weights != NULL)
    {
        f->weights        = user_weights;
        f->weights_owned  = 0;
    }
    else
    {
        /* 自动生成线性递增权重: 1, 2, 3, ..., window_size */
        f->weights        = (float *)malloc(window_size * sizeof(float));
        f->weights_owned  = 1;
        float weight_sum = 0.0f;
        for (size_t i = 0; i < window_size; i++)
        {
            f->weights[i] = (float)(i + 1);
            weight_sum += f->weights[i];
        }
        /* 归一化 */
        for (size_t i = 0; i < window_size; i++)
        {
            f->weights[i] /= weight_sum;
        }
    }
}

/**
 * @brief  释放加权滑动平均滤波器内部缓冲区
 */
void Filter_WMA_DeInit(Filter_WMA_TypeDef *f)
{
    if (f->buffer_owned && f->buffer != NULL)
    {
        free(f->buffer);
        f->buffer = NULL;
    }
    f->buffer_owned = 0;

    if (f->weights_owned && f->weights != NULL)
    {
        free(f->weights);
        f->weights = NULL;
    }
    f->weights_owned = 0;

    f->count        = 0;
    f->index        = 0;
}

/**
 * @brief  加权滑动平均滤波计算
 * @param  f     滤波器实例指针
 * @param  input 当前采样值
 * @return 滤波后的值
 */
float Filter_WMA_Compute(Filter_WMA_TypeDef *f, float input)
{
    f->buffer[f->index] = input;

    f->index++;
    if (f->index >= f->window_size)
    {
        f->index = 0;
    }

    if (f->count < f->window_size)
    {
        f->count++;
    }

    /* 加权求和 */
    float result = 0.0f;
    for (size_t i = 0; i < f->count; i++)
    {
        size_t idx = (f->index >= i + 1)
                         ? (f->index - i - 1)
                         : (f->index + f->window_size - i - 1);
        result += f->buffer[idx] * f->weights[f->count - 1 - i];
    }

    return result;
}

/**
 * @brief  重置加权滑动平均滤波器
 */
void Filter_WMA_Reset(Filter_WMA_TypeDef *f)
{
    for (size_t i = 0; i < f->window_size; i++)
    {
        f->buffer[i] = 0.0f;
    }
    f->index = 0;
    f->count = 0;
}

/*==================================================================
 *  一阶低通滤波 (IIR)
 *==================================================================*/

/**
 * @brief  初始化一阶低通滤波器
 * @param  f     滤波器实例指针
 * @param  alpha 滤波系数 (0~1)，传 0 则使用默认
 */
void Filter_LPF_Init(Filter_LPF_TypeDef *f, float alpha)
{
    f->alpha      = (alpha > 0.0f) ? alpha : FILTER_LPF_DEFAULT_ALPHA;
    f->last_output = 0.0f;
    f->is_first    = 1;
}

/**
 * @brief  一阶低通滤波计算
 * @param  f     滤波器实例指针
 * @param  input 当前采样值
 * @return 滤波后的值
 */
float Filter_LPF_Compute(Filter_LPF_TypeDef *f, float input)
{
    if (f->is_first)
    {
        f->last_output = input;
        f->is_first    = 0;
        return input;
    }

    f->last_output = f->alpha * input + (1.0f - f->alpha) * f->last_output;
    return f->last_output;
}

/**
 * @brief  重置一阶低通滤波器
 */
void Filter_LPF_Reset(Filter_LPF_TypeDef *f)
{
    f->last_output = 0.0f;
    f->is_first    = 1;
}

/**
 * @brief  在线调整滤波系数
 * @param  f     滤波器实例指针
 * @param  alpha 新的滤波系数 (0~1)
 */
void Filter_LPF_SetAlpha(Filter_LPF_TypeDef *f, float alpha)
{
    if (alpha > 0.0f && alpha <= 1.0f)
    {
        f->alpha = alpha;
    }
}

/*==================================================================
 *  中值滤波
 *==================================================================*/

/**
 * @brief  初始化中值滤波器
 * @param  f           滤波器实例指针
 * @param  window_size 窗口大小，传 0 则使用默认
 * @param  user_buffer 用户缓冲区，传 NULL 则内部 malloc
 */
void Filter_Median_Init(Filter_Median_TypeDef *f, size_t window_size,
                        float *user_buffer)
{
    if (window_size == 0) { window_size = FILTER_MEDIAN_DEFAULT_WINDOW; }

    f->window_size = window_size;
    f->index       = 0;
    f->count       = 0;

    if (user_buffer != NULL)
    {
        f->buffer       = user_buffer;
        f->buffer_owned = 0;
    }
    else
    {
        f->buffer       = (float *)calloc(window_size, sizeof(float));
        f->buffer_owned = 1;
    }
}

/**
 * @brief  释放中值滤波器内部缓冲区
 */
void Filter_Median_DeInit(Filter_Median_TypeDef *f)
{
    if (f->buffer_owned && f->buffer != NULL)
    {
        free(f->buffer);
        f->buffer = NULL;
    }
    f->buffer_owned = 0;
    f->count        = 0;
    f->index        = 0;
}

/**
 * @brief  中值滤波计算
 * @param  f     滤波器实例指针
 * @param  input 当前采样值
 * @return 窗口内数据的中值
 */
float Filter_Median_Compute(Filter_Median_TypeDef *f, float input)
{
    f->buffer[f->index] = input;

    f->index++;
    if (f->index >= f->window_size)
    {
        f->index = 0;
    }

    if (f->count < f->window_size)
    {
        f->count++;
    }

    /* 复制当前窗口数据并排序取中值 */
    /* 栈上预分配默认窗口大小的排序缓冲区，大数据量走 malloc */
    float sorted_stack[FILTER_MEDIAN_DEFAULT_WINDOW];
    float *sort_buf;
    if (f->window_size <= FILTER_MEDIAN_DEFAULT_WINDOW)
    {
        sort_buf = sorted_stack;
    }
    else
    {
        sort_buf = (float *)malloc(f->window_size * sizeof(float));
    }

    memcpy(sort_buf, f->buffer, f->count * sizeof(float));
    bubble_sort(sort_buf, f->count);

    float median;
    if (f->count % 2 == 1)
    {
        median = sort_buf[f->count / 2];
    }
    else
    {
        median = (sort_buf[f->count / 2 - 1] + sort_buf[f->count / 2]) * 0.5f;
    }

    if (sort_buf != sorted_stack)
    {
        free(sort_buf);
    }

    return median;
}

/**
 * @brief  重置中值滤波器
 */
void Filter_Median_Reset(Filter_Median_TypeDef *f)
{
    for (size_t i = 0; i < f->window_size; i++)
    {
        f->buffer[i] = 0.0f;
    }
    f->index = 0;
    f->count = 0;
}

/*==================================================================
 *  卡尔曼滤波 (单维)
 *==================================================================*/

/**
 * @brief  初始化卡尔曼滤波器
 * @param  f              滤波器实例指针
 * @param  q              过程噪声协方差，传 0 则使用默认
 * @param  r              测量噪声协方差，传 0 则使用默认
 * @param  initial_value  初始状态估计值
 */
void Filter_Kalman_Init(Filter_Kalman_TypeDef *f, float q, float r,
                        float initial_value)
{
    f->q = (q > 0.0f) ? q : FILTER_KALMAN_DEFAULT_Q;
    f->r = (r > 0.0f) ? r : FILTER_KALMAN_DEFAULT_R;
    f->x = initial_value;
    f->p = 1.0f;        /* 初始估计误差协方差 */
    f->k = 0.0f;
    f->is_first = 0;
}

/**
 * @brief  卡尔曼滤波计算
 * @param  f           滤波器实例指针
 * @param  measurement 当前测量值
 * @return 最优估计值
 */
float Filter_Kalman_Compute(Filter_Kalman_TypeDef *f, float measurement)
{
    /* ---- 预测 (先验) ---- */
    /* 状态预测: x = x (假设系统无控制输入，状态不变) */
    /* 协方差预测: p = p + q */
    f->p = f->p + f->q;

    /* ---- 更新 (后验) ---- */
    /* 卡尔曼增益: k = p / (p + r) */
    f->k = f->p / (f->p + f->r);

    /* 状态更新: x = x + k * (z - x) */
    f->x = f->x + f->k * (measurement - f->x);

    /* 协方差更新: p = (1 - k) * p */
    f->p = (1.0f - f->k) * f->p;

    return f->x;
}

/**
 * @brief  重置卡尔曼滤波器
 * @param  f              滤波器实例指针
 * @param  initial_value  新的初始状态估计值
 */
void Filter_Kalman_Reset(Filter_Kalman_TypeDef *f, float initial_value)
{
    f->x = initial_value;
    f->p = 1.0f;
    f->k = 0.0f;
}

/**
 * @brief  在线调整卡尔曼噪声参数
 * @param  f 滤波器实例指针
 * @param  q 过程噪声协方差 (q 越大越信任测量值)
 * @param  r 测量噪声协方差 (r 越大越信任模型预测)
 */
void Filter_Kalman_SetQR(Filter_Kalman_TypeDef *f, float q, float r)
{
    if (q > 0.0f) { f->q = q; }
    if (r > 0.0f) { f->r = r; }
}
