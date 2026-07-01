/* SPDX-License-Identifier: MIT */
#ifndef LIVERPOOL_CLK_H
#define LIVERPOOL_CLK_H

struct amdgpu_device;

#ifdef CONFIG_DRM_AMDGPU_CIK
int liverpool_clk_force_max(struct amdgpu_device *adev);
#else
static inline int liverpool_clk_force_max(struct amdgpu_device *adev)
{ return 0; }
#endif

#endif /* LIVERPOOL_CLK_H */
