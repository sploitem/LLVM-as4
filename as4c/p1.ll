; ModuleID = 'Python compiler'
source_filename = "Python compiler"

define float @foo() {
entry:
  %sphere_surf_area = alloca float
  %sphere_vol = alloca float
  %circle_circum = alloca float
  %circle_area = alloca float
  %r = alloca float
  %b = alloca float
  %a = alloca float
  %pi = alloca float
  store float 0x400921CAC0000000, float* %pi
  store float 3.000000e+00, float* %a
  store float 2.000000e+00, float* %b
  %b1 = load float, float* %b
  %b2 = load float, float* %b
  %b3 = load float, float* %b
  %a4 = load float, float* %a
  %addtmp = fadd float %a4, %b3
  %multmp = fmul float %addtmp, %b2
  %subtmp = fsub float %multmp, %b1
  store float %subtmp, float* %r
  %r5 = load float, float* %r
  %r6 = load float, float* %r
  %pi7 = load float, float* %pi
  %multmp8 = fmul float %pi7, %r6
  %multmp9 = fmul float %multmp8, %r5
  store float %multmp9, float* %circle_area
  %r10 = load float, float* %r
  %pi11 = load float, float* %pi
  %multmp12 = fmul float 2.000000e+00, %pi11
  %multmp13 = fmul float %multmp12, %r10
  store float %multmp13, float* %circle_circum
  %r14 = load float, float* %r
  %r15 = load float, float* %r
  %r16 = load float, float* %r
  %pi17 = load float, float* %pi
  %multmp18 = fmul float 7.500000e-01, %pi17
  %multmp19 = fmul float %multmp18, %r16
  %multmp20 = fmul float %multmp19, %r15
  %multmp21 = fmul float %multmp20, %r14
  store float %multmp21, float* %sphere_vol
  %r22 = load float, float* %r
  %r23 = load float, float* %r
  %pi24 = load float, float* %pi
  %multmp25 = fmul float 4.000000e+00, %pi24
  %multmp26 = fmul float %multmp25, %r23
  %multmp27 = fmul float %multmp26, %r22
  store float %multmp27, float* %sphere_surf_area
  ret void

; uselistorder directives
  uselistorder i32 1, { 7, 6, 5, 4, 3, 2, 1, 0 }
}
