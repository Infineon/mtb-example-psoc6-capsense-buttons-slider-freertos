/******************************************************************************
* File Name: led_task.c
*
* Description: This file contains the task that handles led.
*
* Related Document: README.md
*
*******************************************************************************
 * (c) 2019-2025, Infineon Technologies AG, or an affiliate of Infineon
 * Technologies AG. All rights reserved.
 * This software, associated documentation and materials ("Software") is
 * owned by Infineon Technologies AG or one of its affiliates ("Infineon")
 * and is protected by and subject to worldwide patent protection, worldwide
 * copyright laws, and international treaty provisions. Therefore, you may use
 * this Software only as provided in the license agreement accompanying the
 * software package from which you obtained this Software. If no license
 * agreement applies, then any use, reproduction, modification, translation, or
 * compilation of this Software is prohibited without the express written
 * permission of Infineon.
 *
 * Disclaimer: UNLESS OTHERWISE EXPRESSLY AGREED WITH INFINEON, THIS SOFTWARE
 * IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING, BUT NOT LIMITED TO, ALL WARRANTIES OF NON-INFRINGEMENT OF
 * THIRD-PARTY RIGHTS AND IMPLIED WARRANTIES SUCH AS WARRANTIES OF FITNESS FOR A
 * SPECIFIC USE/PURPOSE OR MERCHANTABILITY.
 * Infineon reserves the right to make changes to the Software without notice.
 * You are responsible for properly designing, programming, and testing the
 * functionality and safety of your intended application of the Software, as
 * well as complying with any legal requirements related to its use. Infineon
 * does not guarantee that the Software will be free from intrusion, data theft
 * or loss, or other breaches ("Security Breaches"), and Infineon shall have
 * no liability arising out of any Security Breaches. Unless otherwise
 * explicitly approved by Infineon, the Software may not be used in any
 * application where a failure of the Product or any consequences of the use
 * thereof can reasonably be expected to result in personal injury.
*******************************************************************************/


/*******************************************************************************
 * Header file includes
 ******************************************************************************/
#include "led_task.h"
#include "cybsp.h"
#include "cyhal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "cycfg.h"


/*******************************************************************************
* Global constants
*******************************************************************************/
#define PWM_LED_FREQ_HZ     (1000000u)  /* in Hz */
#define GET_DUTY_CYCLE(x)   (100 - x)   /* subtracting from 100 since the LED 
                                         * is connected in active low 
                                         * configuration
                                         */


/*******************************************************************************
 * Global variable
 ******************************************************************************/
/* Queue handle used for LED data */
QueueHandle_t led_command_data_q;


/*******************************************************************************
* Function Name: task_led
********************************************************************************
* Summary:
*  Task that controls the LED.
*
* Parameters:
*  void *param : Task parameter defined during task creation (unused)
*
*******************************************************************************/
void task_led(void* param)
{
    cyhal_pwm_t pwm_led;
    bool led_on = true;
    BaseType_t rtos_api_result;
    led_command_data_t led_cmd_data;

    /* Suppress warning for unused parameter */
    (void)param;

    /* Initialize a PWM resource for driving an LED. */
    cyhal_pwm_init(&pwm_led, CYBSP_USER_LED, NULL);
    cyhal_pwm_set_duty_cycle(&pwm_led, GET_DUTY_CYCLE(LED_MAX_BRIGHTNESS),
                             PWM_LED_FREQ_HZ);
    cyhal_pwm_start(&pwm_led);

    /* Repeatedly running part of the task */
    for(;;)
    {
        /* Block until a command has been received over queue */
        rtos_api_result = xQueueReceive(led_command_data_q, &led_cmd_data,
                            portMAX_DELAY);

        /* Command has been received from queue */
        if(rtos_api_result == pdTRUE)
        {
            switch(led_cmd_data.command)
            {
                /* Turn on the LED. */
                case LED_TURN_ON:
                {
                    if (!led_on)
                    {
                        /* Start PWM to turn the LED on */
                        cyhal_pwm_start(&pwm_led);
                        led_on = true;
                        led_cmd_data.brightness=LED_MAX_BRIGHTNESS;
                    }
                    break;
                }
                /* Turn off LED */
                case LED_TURN_OFF:
                {
                    if(led_on)
                    {
                        /* Stop PWM to turn the LED off */
                        cyhal_pwm_stop(&pwm_led);
                        led_on = false;
                        led_cmd_data.brightness=0;
                    }
                    break;
                }
                /* Update LED brightness */
                case LED_UPDATE_BRIGHTNESS:
                {
                    if ((led_on) || ((!led_on) && (led_cmd_data.brightness>0)))
                    {
                        /* Start PWM to turn the LED on */
                        cyhal_pwm_start(&pwm_led);
                        uint32_t brightness = (led_cmd_data.brightness < LED_MIN_BRIGHTNESS) ?
                                               LED_MIN_BRIGHTNESS : led_cmd_data.brightness;

                        /* Drive the LED with brightness */
                        cyhal_pwm_set_duty_cycle(&pwm_led, GET_DUTY_CYCLE(brightness),
                                                 PWM_LED_FREQ_HZ);
                        led_on = true;
                    }
                    break;
                }
                /* Invalid command */
                default:
                {
                    /* Handle invalid command here */
                    break;
                }
            }
        }

        /* Task has timed out and received no data during an interval of
         * portMAXDELAY ticks.
         */
        else
        {
            /* Handle timeout here */
        }
    }
}


/* END OF FILE [] */
