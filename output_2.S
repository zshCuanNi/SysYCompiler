f_func1 [3]
	var t0
	var t1
	var t2
	var t3
	t3 = p2 == 0
	if t3 != 0 goto l2
	goto l1
l2:
	t0 = p0 * p1
	return t0
	goto l3
l1:
	t1 = p1 - p2
	param p0
	param t1
	param 0
	t2 = call f_func1
	return t2
l3:
	return 0
end f_func1

f_func2 [2]
	var t0
	var t1
	if p1 != 0 goto l6
	goto l5
l6:
	t0 = p0 % p1
	param t0
	param 0
	t1 = call f_func2
	return t1
	goto l7
l5:
	return p0
l7:
	return 0
end f_func2

f_func3 [2]
	var t0
	var t1
	var t2
	var t3
	t3 = p1 == 0
	if t3 != 0 goto l10
	goto l9
l10:
	t0 = p0 + 1
	return t0
	goto l11
l9:
	t1 = p0 + p1
	param t1
	param 0
	t2 = call f_func3
	return t2
l11:
	return 0
end f_func3

f_func4 [3]
	if p0 != 0 goto l14
	goto l13
l14:
	return p1
	goto l15
l13:
	return p2
l15:
	return 0
end f_func4

f_func5 [1]
	var t0
	t0 = -p0
	return t0
end f_func5

f_func6 [2]
	if p0 == 0 goto l17
	if p1 != 0 goto l18
	goto l17
l18:
	return 1
	goto l19
l17:
	return 0
l19:
	return 0
end f_func6

f_func7 [1]
	var t0
	t0 = !p0
	if t0 != 0 goto l22
	goto l21
l22:
	return 1
	goto l23
l21:
	return 0
l23:
	return 0
end f_func7

f_main [0]
	var t0
	var T0
	var t1
	var T1
	var t2
	var T2
	var t3
	var T3
	var 40 T4
	var T5
	var t4
	var t5
	var t6
	var t7
	var t8
	var t9
	var t10
	var t11
	var t12
	var t13
	var t14
	var t15
	var t16
	var t17
	var t18
	var t19
	var t20
	var t21
	var t22
	var t23
	var t24
	var t25
	var t26
	var t27
	var t28
	var t29
	var t30
	var t31
	var t32
	var t33
	var t34
	var t35
	var t36
	var t37
	var t38
	var t39
	var t40
	var t41
	var t42
	var t43
	var t44
	var t45
	var t46
	var t47
	var t48
	var t49
	var t50
	var t51
	var t52
	var t53
	var t54
	var t55
	var t56
	var t57
	var t58
	var t59
	var t60
	var T6
	t0 = call f_getint
	T0 = t0
	t1 = call f_getint
	T1 = t1
	t2 = call f_getint
	T2 = t2
	t3 = call f_getint
	T3 = t3
	T5 = 0
l24:
	t6 = T5 < 10
	if t6 == 0 goto l25
l26:
	t5 = call f_getint
	t4 = T5 * 4
	T4[t4] = t5
	T5 = T5 + 1
	goto l24
l25:
	param T0
	t7 = call f_func7
	param T1
	t8 = call f_func5
	param t7
	param t8
	t9 = call f_func6
	param t9
	param T2
	t10 = call f_func2
	param t10
	param T3
	t11 = call f_func3
	param t11
	t31 = call f_func5
	t32 = T4[0]
	t12 = T4[4]
	param t12
	t19 = call f_func5
	t14 = T4[8]
	t13 = T4[12]
	param t13
	t15 = call f_func7
	param t14
	param t15
	t20 = call f_func6
	t17 = T4[16]
	t16 = T4[20]
	param t16
	t18 = call f_func7
	param t17
	param t18
	t21 = call f_func2
	param t19
	param t20
	param t21
	t22 = call f_func4
	t23 = T4[24]
	param t22
	param t23
	t24 = call f_func3
	t25 = T4[28]
	param t24
	param t25
	t29 = call f_func2
	t27 = T4[32]
	t26 = T4[36]
	param t26
	t28 = call f_func7
	param t27
	param t28
	t30 = call f_func3
	param t29
	param t30
	param T0
	t33 = call f_func1
	param t31
	param t32
	param t33
	t36 = call f_func4
	param T2
	t34 = call f_func7
	param t34
	param T3
	t35 = call f_func3
	param T1
	param t35
	t37 = call f_func2
	param t36
	param t37
	t38 = call f_func3
	t39 = T4[0]
	t40 = T4[4]
	param t38
	param t39
	param t40
	t41 = call f_func1
	t42 = T4[8]
	param t41
	param t42
	t57 = call f_func2
	t58 = T4[12]
	t44 = T4[16]
	t43 = T4[20]
	param t43
	t45 = call f_func5
	param t44
	param t45
	t47 = call f_func3
	t46 = T4[24]
	param t46
	t48 = call f_func5
	param t47
	param t48
	t50 = call f_func2
	t51 = T4[28]
	t49 = T4[32]
	param t49
	t52 = call f_func7
	param t50
	param t51
	param t52
	t54 = call f_func1
	t53 = T4[36]
	param t53
	t55 = call f_func5
	param t54
	param t55
	t56 = call f_func2
	param t56
	param T0
	t59 = call f_func3
	param t57
	param t58
	param t59
	t60 = call f_func1
	T6 = t60
	return T6
end f_main

