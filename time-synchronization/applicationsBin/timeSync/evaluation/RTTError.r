delay1 <- function (t0,t1,t2,t3) {
	d = (trunc(t3) - trunc(t0)) - (trunc(t2) - trunc(t1))
	return (d)
}

delay2 <- function (t0,t1,t2,t3) {
	d = trunc(t1) + (trunc(t3) - trunc(t0)) - trunc(t2)
	return (d)
}

delayr <- function (t0,t1,t2,t3) {
	d = (t3 - t0) - t2 + t1
	return (d)
}

t0 = 1
t1 = 6.9
t2 = 7.1
t3 = 12

d1 = delay1(t0,t1,t2,t3)
d2 = delay2(t0,t1,t2,t3)
dr = delayr(t0,t1,t2,t3)

cat("d1: ", d1, "\n")
cat("d2: ", d2, "\n")
cat("dr: ", dr, "\n")
