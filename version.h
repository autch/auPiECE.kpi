#pragma once

// KbMedia Player は同じ拡張子のプラグインをこの値の大きい順に優先するので、
// 上位 16bit に muslib、下位 16bit に kpi のバージョンを入れる 1.x の形式を続ける。
#define AUPIECE_MUSLIB_VERSION	((2 << 8) | 3)		// 2.03 (libmuslib)
#define AUPIECE_KPI_LAYER_VERSION	((2 << 8) | 0)	// 2.00
#define AUPIECE_KPI_VERSION		(((AUPIECE_MUSLIB_VERSION) << 16) | (AUPIECE_KPI_LAYER_VERSION))
