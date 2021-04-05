#include <os/kernel.h>
#include <drivers/regs_util.h>
#include <drivers/timer_port.h>

/* systick��ʱ����С��ʱʱ�䣨��SysTick�ж��ӳٵ��ʱ�䣬��ǰ����Ϊ16000��cycles�� */
#define SYSTICK_MIN_TIMEOUT         (16000)

/* SYSTICK�Ĵ������� */
#define SYSTICK_BASE                0xE000E010

/* SYSTICK MAX TIMEOUT */
#define SYSTICK_MAX_COUNT_CYCLES    BIT(24)

/* SYSTICK CSR�Ĵ������ֶ� */
#define SYSTICK_R_CSR               REG_ENTITY(0x0000)
#define SYSTICK_F_COUNTFLAG         REG_FIELD(0x0000, 16, 16)
#define SYSTICK_F_CLKSOURCE         REG_FIELD(0x0000, 2, 2)
#define SYSTICK_F_TICKINT           REG_FIELD(0x0000, 1, 1)
#define SYSTICK_F_ENABLE            REG_FIELD(0x0000, 0, 0)

/* SYSTICK RELOAD�Ĵ��� */ 
#define SYSTICK_R_RELOAD            REG_ENTITY(0x0004)
#define SYSTICK_F_RELOAD            REG_FIELD(0x0004, 0, 23)

/* SYSTICK CVR�Ĵ��� */    
#define SYSTICK_R_CVR               REG_ENTITY(0x0008)
#define SYSTICK_F_CVR               REG_FIELD(0x0008, 0, 23)

/* SYSTICK CALIB�Ĵ��� */  
#define SYSTICK_R_CALIB             REG_ENTITY(0x000C)
#define SYSTICK_F_NOREF             REG_FIELD(0x000C, 31, 31)
#define SYSTICK_F_SKEW              REG_FIELD(0x000C, 30, 30)
#define SYSTICK_F_TENMS             REG_FIELD(0x000C, 0, 23)

/* ICSR */
#define CORTEX_M_ICSR                   0xE000ED04
#define CORTEX_SYSTICK_IRQ_PENDSET      BIT(26)

struct drv_systick_ctx_s {
    ktime_tick_t overflow;
    ktime_tick_t expiry;
};

static struct drv_systick_ctx_s drv_ctx;

void cortex_m_systick_init(void)
{
    /* ��ʼ��Systick */
    REG_WRITE_FIELD(SYSTICK_BASE, SYSTICK_R_RELOAD, SYSTICK_MAX_COUNT_CYCLES - 1);
    REG_WRITE_FIELD(SYSTICK_BASE, SYSTICK_R_CVR, 0);

    REG_WRITE_FIELDS_NO_READBACK(SYSTICK_BASE,
                                 SYSTICK_F_CLKSOURCE, 1,
                                 SYSTICK_F_TICKINT, 1,
                                 SYSTICK_F_ENABLE, 1);

    /* ��ΪSystick��COUNT���¼�����ʵ�ʾ�����ʱ��Ϊoverflow + RELOAD - COUNT��
     * ��ˣ����ǽ�RELOAD����overflow�Լ��ٶ��������
     */
    drv_ctx.overflow = SYSTICK_MAX_COUNT_CYCLES;
}

ktime_tick_t drv_ktime_tick_get(void)
{
    ktime_tick_t overflow;
    uint32_t cvr1, cvr2, countflag;
    uint32_t reload;

    int key = irq_lock();

    /* �����ζ�ȡcvr��ֵȷ��countflag��cvr֮���һ���� */
    cvr1 = REG_READ_FIELD(SYSTICK_BASE, SYSTICK_R_CVR);
    countflag = REG_READ_FIELD(SYSTICK_BASE, SYSTICK_F_COUNTFLAG);
    cvr2 = REG_READ_FIELD(SYSTICK_BASE, SYSTICK_R_CVR);

    overflow = drv_ctx.overflow;

    /* ������������ */
    if (cvr2 > cvr1 || countflag) {
        /* ��ȡCSR�Ĵ������COUNTFLAG */
        (void)REG_READ_FIELD(SYSTICK_BASE, SYSTICK_R_CSR);

        reload = REG_READ_FIELD(SYSTICK_BASE, SYSTICK_R_RELOAD);

        overflow += reload + 1;
        drv_ctx.overflow = overflow;
    }

    irq_unlock(key);

    return overflow - cvr2;
}

static void systick_reset_reload(uint32_t reload)
{
    uint32_t cvr1, cvr2, countflag;
    uint32_t old_reload;
    ktime_tick_t overflow;

    /* ����ɵ�reloadֵ�������㵱ǰʱ�� */
    old_reload = REG_READ_FIELD(SYSTICK_BASE, SYSTICK_R_RELOAD);
    /* �����µ�reload��Ϊ��ʱʱ�� */
    REG_WRITE_FIELD(SYSTICK_BASE, SYSTICK_R_RELOAD, reload - 1);

    /* ��ȡ��ǰCVR��״̬�������㿪���µ�reload��ʱ */
    cvr1 = REG_READ_FIELD(SYSTICK_BASE, SYSTICK_R_CVR);
    countflag = REG_READ_FIELD(SYSTICK_BASE, SYSTICK_F_COUNTFLAG);
    cvr2 = REG_READ_FIELD(SYSTICK_BASE, SYSTICK_R_CVR);
    /* ��������ֵ��CVR���� */
    REG_WRITE_FIELD(SYSTICK_BASE, SYSTICK_R_CVR, cvr2);

    /* ��ȡCSR�Ĵ���ȷ�����COUNTFLAG */
    (void)REG_READ_FIELD(SYSTICK_BASE, SYSTICK_R_CSR);

    /* ��������reloadǰ������ʱ�� */
    overflow = drv_ctx.overflow;
    if (cvr2 > cvr1 || countflag) {
        overflow += old_reload + 1;
    }

    overflow -= cvr2;

    /* �����µ�RELOAD
     * �µĳ�ʱ��CVR����ʱ��Ч����cvr2��CVR����֮����2��cycles����ʱ������һ�����
     */
    overflow += reload + 2;

    drv_ctx.overflow = overflow;
}

void drv_ktimer_set_expiry(ktime_tick_t expiry)
{
    int key;
    uint32_t reload;
    ktime_tick_t timeout, now;

    key = irq_lock();

    /* �޳�ʱ */
    if (expiry == 0) {
        reload = SYSTICK_MAX_COUNT_CYCLES;
        drv_ctx.expiry = INT64_MAX;
    }
    /* ���ó�ʱʱ�� */
    else {
        now = drv_ktime_tick_get();
        timeout = expiry - now;
        drv_ctx.expiry = expiry;

        /* ����ʱʱ��С��128��cyclesʱ�䣬������������ʱ
         * 128��cycles��֤ʵ�ʴ���ʱ���Ѿ���ʱ���õĵ���ֵ
         */
        if (timeout < 128) {
            /* Pending Systick IRQ */
            REG_WRITE_ENTITY(CORTEX_M_ICSR, CORTEX_SYSTICK_IRQ_PENDSET);
            irq_unlock(key);
            return;
        }

        if (timeout >= SYSTICK_MAX_COUNT_CYCLES * 2) {
            reload = SYSTICK_MAX_COUNT_CYCLES;
        } else if ((uint32_t)timeout > SYSTICK_MAX_COUNT_CYCLES) {
            reload = (uint32_t)timeout / 2;
        } else if ((uint32_t)timeout < SYSTICK_MIN_TIMEOUT) {
            reload = SYSTICK_MIN_TIMEOUT;
        } else {
            reload = (uint32_t)timeout;
        }
    }

    systick_reset_reload(reload);

    irq_unlock(key);
}

/* Systick�ж� */
void SysTick_Handler(void)
{
    ktime_tick_t now, expiry, timeout;
    int key;

    key = irq_lock();

    /* ���㳬ʱʱ�� */
    expiry = drv_ctx.expiry;
    now = drv_ktime_tick_get();
    timeout = expiry - now;

    if (timeout > 0) {
        /* ��timeoutС�ڿ�systick�ɼ�����ʱ��ʱ��������reload */
        if (timeout <= SYSTICK_MAX_COUNT_CYCLES) {
            systick_reset_reload((uint32_t)timeout);
        }

        irq_unlock(key);
        return;
    }

    irq_unlock(key);

    /* ����ʱ */
    sys_ktimer_timeout_check(now);
}
