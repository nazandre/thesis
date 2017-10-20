convertToBitrateAndPrintStats <- function (time)
{
	rate <- (21*8.0)/(time*10^(-3))
	rate <- rate / 1000
	cat("\tMean: ", mean(rate), "\n")
	cat("\tStandard-deviation: ", sd(rate), "\n")
}	

n <- 50000
# unit : ms

# OLD
#sparseTime <- rnorm(n, mean = 5.9665, sd = 0.23)
#intermediateTime <- rnorm(n, mean = 5.99, sd = 0.28)
#compactTime <- rnorm(n, mean = 6.08, sd = 0.33)

# New 
sparseTime <- rnorm(n, mean = 5.973, sd = 0.14)
intermediateTime <- rnorm(n, mean = 5.988, sd = 0.20)
compactTime <- rnorm(n, mean = 6.075, sd = 0.25)

cat("Sparse:\n")
convertToBitrateAndPrintStats(sparseTime)

cat("Intermediate:\n")
convertToBitrateAndPrintStats(intermediateTime)

cat("Compact:\n")
convertToBitrateAndPrintStats(compactTime)