#pragma once

void wifi_irqRxStart();
void wifi_irqRxEnd();
void wifi_initRx();

typedef void(*wifi_rx_done_callback_t)(void* arg);

void wifi_setRxDoneCallback(wifi_rx_done_callback_t callback, void* arg);