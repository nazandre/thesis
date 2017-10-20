file <- commandArgs(TRUE)[1]

clockData <- read.table(file)

# 1: sim time t (us)
# 2: Gmax(L(t)) - Gmin(L(t)) (us)

simTime <- clockData[,1]/1000
diff <- clockData[,2]/1000

diff <- diff[6:length(diff)]

#print(summary(abs(diff)))

cat("mean:", mean(diff), "\n")
cat("sd:", sd(diff), "\n")
cat("min:", min(diff), "\n")
cat("max:", max(diff), "\n")
