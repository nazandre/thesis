file <- commandArgs(TRUE)[1]

outputDistribution <- "distribution.dat"
outputSummary <- "summary.dat"

clockData <- read.table(file)

# 1: sim time t (us)
# 2: Gmax(L(t)) - Gmin(L(t)) (us)

simTime <- clockData[,1]/1000
diff <- clockData[,2]/1000

# ignore the five first 30min (convergence time)
minToMS <- function(t) {
	return(t*1000*60)
}

filter <- minToMS(60) + minToMS(30)
simTime <- simTime[ simTime > filter ]
sizeToKeep <- length(simTime)

diffIgnore <- tail(diff,sizeToKeep)

#cat ("length : ", length(diff), "\n")

########### # Summary

mySummary <- matrix(nrow=1,ncol=5)
mySummary[1,1] = mean(diffIgnore)
mySummary[1,2] = min(diffIgnore)
mySummary[1,3] = max(diffIgnore)
mySummary[1,4] = sd(diffIgnore)

# convergence:
convCriteria = mySummary[1,1]*(1+0.25)
convIndex = min(which(diffIgnore <= convCriteria))
mySummary[1,5] = simTime[convIndex]

#cat("mean:", mean(diff), "\n")
#cat("sd:", sd(diff), "\n")
#cat("min:", min(diff), "\n")
#cat("max:", max(diff), "\n")

write.table(mySummary, quote = FALSE, file = outputSummary, sep=" ", col.names=FALSE, row.names = FALSE)

########### # Distribution

a <- table(diff)
f <- as.data.frame(a)
x <- c(sort(unique(diff)))
y <-f$Freq/length(diff)*100
   
total <- sum(y)
#cat("total: ", total, "\n")
   
dataToExport <- data.frame(x,y)
da2 <-as.matrix(dataToExport)
da3 <- matrix(da2, ncol=ncol(da2),dimnames = NULL)
names(da3) <- NULL

write.table(da3,  quote = FALSE, file = outputDistribution, sep=" ",col.names=FALSE,row.names = FALSE)