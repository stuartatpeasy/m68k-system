#
# irq_task.S: context-switching interrupt handler
#

.text

next_task:
			##################################
			# save this task's machine state #
			##################################
											# *--sp = a0
			move.l	%a0, %sp@-
											# a0 = &g_current_task
			move.l	g_current_task, %a0
											# a0 = g_current_task
			move.l	%a0@, %a0
											# a0 = &(g_current_task.sr)
			adda.w	#64, %a0

			# save d0-d7, a0-a6 and USP
			#								# repeat: *--a0 = a[6-0],d[7-0]
			movem.l	%d0-%d7/%a0-%a6, %a0@-
											# a0 = &(g_current_task.a[0])
			adda.l	#32, %a0
											# g_current_task.a[0] = *sp++
			move.l	%sp@+, %a0@
											# a0 = &(g_current_task.a[7])
			adda.l	#28, %a0
											# a1 = usp
			move.l	%usp, %a1
											# g_current_task.a[7] = a1
			move.l	%a1, %a0@

			# save SR and PC
			#								# a0 = &(g_current_task.sr) [lower word]
			addq.l	#6, %a0
											# (u16) *sp = g_current_task.sr
			move.w	%sp, %a0@+
											# *(sp + 2) = g_current_task.pc
			move.l	%sp@(2), %a0@+

			# advance taskptr to next task
			#								# a1 = g_current_task->next
			move.l	%a0@, %a1
											# a0 = &g_current_task
			move.l	g_current_task, %a0
											# g_current_task = a1
			move.l	%a1, %a0@

			############################
			# restore new task's state #
			############################

			# restore USP, SR and PC
			#								# a1 = &(next.a[7])
			adda.l	#60, %a1
											# usp = next.a[7]
			move.l	%a1, %usp
											# a1 = &(next.sr)
			addq.l	#6, %a1
											# (u16) *sp = next.sr
											# a1 = &(next.pc)
			move.w	%a1@+, %sp@
											# *(sp + 2) = a1
			move.l	%a1@, %sp@(2)
											# a1 = &(next.d[1])
			suba.l	#64, %a1
											# a0 = a1
			move.l	%a1, %d0

			# restore d0-d7, a0-a6
			#
			movem.l	%a0@+, %d0-%d7/%a0-%a6

			rte

