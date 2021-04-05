                        .thumb
                        .syntax     unified
                        .text

                        .global     PendSV_Handler
                        .type       PendSV_Handler, %function
PendSV_Handler:
                       /* PendSV�쳣�������ͨ������α����쳣����������ʵ����ռ���жϵ��̵߳�CPU
                        * ������ռ�������ֶ��Ļظ����ж��̵߳�������
                        */

                       /* �������쳣ʱ����ǰ��LR�뱻ѹջ��xPSRָʾ�˱��жϵ��߳�ĳЩ���ú�״̬��
                        *  LR:
                        *  BIT3: �����λ������ʱ��ָʾ�쳣���ص�Threadģʽ�����򷵻ص�Handlerģʽ��PendSV�쳣����ʱ�����λӦ��Ϊ1
                        *  BIT2: �����λ������ʱ��ָʾ�쳣����ʹ��PSP�ָ��߳������ģ�����ʹ��MSP
                        *  
                        *  ��ѹջ��xPSR:
                        *  BIT24: �����λ������ʱ���쳣���غ����Thumbģʽ�����򷵻ص�ARMģʽ
                        *  BIT9: �����λ������ʱ����ʾջ�ѿ���8Byte���룬��ָʾ�ڵ����߳������ĺ�SP+4
                        */

                        /* ����LR��״̬ */
                        MOV         R0, LR

                        /* ������Ҫ�����쳣�����ֳ���Ҫʹ�õĿռ䣬����ȡ����ջ�� */

                        /* ���LR BIT2ȷ���쳣����ʹ��PSP����MSP */
                        LSLS        R1, R0, #29
                        BMI         insert_fake_exc_return_context_space_use_psp

                        /* ��Ϊ��ǰ����Handlerģʽ��������ǿ���ֱ��ʹ��SP������MSP */
                        SUB         SP, #32
                        MOV         R2, SP
                        B           insert_fake_exc_return_context_space_endif

insert_fake_exc_return_context_space_use_psp:
                        /* ��Handlerģʽ�£���Ҫʹ��MRS��MSRָ�����PSP */
                        MRS         R2, PSP
                        SUBS        R2, #32
                        MSR         PSP, R2
insert_fake_exc_return_context_space_endif:

                        /* �������ö�ջ�з��غ��λ��Ϊpendsv_exc_return_handler */
                        ADR         R0, pendsv_exc_return_handler
                        STR         R0, [R2, #24]

                        /* �������ö�ջ��xPSR��ʹ�쳣���غ����Thumbģʽ���ָ���ǰ��IPSR״̬ */
                        MOVS        R1, #1
                        LSLS        R1, #24
                        /* ��ȡ�����������е�IPSR */
                        LDR         R0, [R2, #60]
                        LSLS        R0, #26
                        LSRS        R0, #26
                        /* ����xPSRΪBIT24 + IPSR */
                        ORRS        R1, R0
                        STR         R1, [R2, #28]

                        /* �쳣���أ�����pendsv_exc_return_handler */
                        BX          LR
                        NOP

pendsv_exc_return_handler:
                        /* ���õ��ȳ��� */
                        BL          k_schedule

                       /* ����ռ����ִ�н���֮��������Ҫ�ָ���ռǰ��������
                        * ���ǻ���Ҫ���xPSR��BIT9��ȷ���ָ�֮���ջ��λ��
                        */

                        /* ���ǽ�R3ָ���ָ̻߳���ջ����ǰ�����ֵ�λ�ã����ڴ��R4��PC */
                        ADD         R3, SP, #24

                        /* ��ȡxPSR */
                        LDR         R0, [SP, #28]

                        /* �ж�xPSR��BIT9������λ��ջ����Ҫ+4 */
                        LSLS        R1, R0, #22
                        LSRS        R1, #31
                        LSLS        R1, #2
                        ADDS        R3, R1

                        /* ��ȡ�̵߳�PC��������xPSR��BIT24����Thumb����ARMģʽ */
                        LDR         R1, [SP, #24]
                        LSLS        R2, R0, #7
                        LSRS        R2, #31
                        ORRS        R1, R2

                       /* ����������Ҫ��R0-R12��LR��SP�ָ�֮�����ִ�з���
                        * ��ʱ������û�п���ʹ�õļĴ�������PC��
                        * �����Ҫ��PC����ջ�У�ʹ��POPָ����лָ�
                        * ͬʱ����Ҳ�޷��ڻָ�R0-R3֮���ٱ���ջ����λ��
                        * ������Ǳ���ʹ��R4��¼ջ������˻���Ҫ��R4������
                        */
                        STR         R1, [R3, #4]
                        STR         R4, [R3, #0]
                        MOV         R4, R3

                       /* �ָ�R12��LR��xPSR
                        * Ϊ�˼���M0�����ǲ�ʹ��LDRDָ�����ָ�R12��LR
                        */
                        MSR         APSR_nzcvq, R0
                        ADD         R0, SP, #16
                        LDM         R0!, {R1-R2}
                        MOV         R12, R1
                        MOV         LR, R2

                        /* �ָ�R0-R3��SP */
                        POP         {R0-R3}
                        MOV         SP, R4

                        /* ���ر���ռ���߳� */
                        POP         {R4, PC}
