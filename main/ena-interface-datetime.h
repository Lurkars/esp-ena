#ifndef _ena_INTERFACE_DATETIME_H_
#define _ena_INTERFACE_DATETIME_H_

typedef enum
{
    ENA_INTERFACE_DATETIME_STATE_YEAR = 0,
    ENA_INTERFACE_DATETIME_STATE_MONTH,
    ENA_INTERFACE_DATETIME_STATE_DAY,
    ENA_INTERFACE_DATETIME_STATE_HOUR,
    ENA_INTERFACE_DATETIME_STATE_MINUTE,
    ENA_INTERFACE_DATETIME_STATE_SECONDS,
} ena_inerface_datetime_state;

void ena_interface_datetime_start(void);

int ena_interface_datetime_state(void);

#endif