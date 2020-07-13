#ifndef _ena_INTERFACE_MENU_H_
#define _ena_INTERFACE_MENU_H_

typedef enum
{
    ENA_INTERFACE_MENU_STATE_IDLE = 0,
    ENA_INTERFACE_MENU_STATE_SELECT_TIME,
    ENA_INTERFACE_MENU_STATE_SELECT_INFO,
} ena_interface_menu_state;

void ena_interface_menu_start(void);

int ena_interface_menu_get_state(void);

#endif