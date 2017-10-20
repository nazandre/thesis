file <- commandArgs(TRUE)[1]

clockData <- read.table(file)

# 1: id
# 2: simTime
# 3: realGlobalTime (master global time) 
# 4: estimatedGlobalTime (msg)
# 5: localTime
# 6: local global time
# 7: distance

estimated <- clockData[,4]/1000
global <- clockData[,6]/1000
error <- (estimatedGlobal - realGlobal)

#print(summary(abs(error)))
#print(mean(error))
#print(sd(error))

a <- table(error)
f <- as.data.frame(a)
x <- c(sort(unique(error)))
y <-f$Freq/length(error)*100

total <- sum(y)
#cat("total: ", total, "\n")

dataToExport <- data.frame(x,y)
da2 <-as.matrix(dataToExport)
da3 <- matrix(da2, ncol=ncol(da2),dimnames = NULL)
names(da3) <- NULL

write.table(da3,  quote = FALSE, file = "tm-error/synchronization-error-density.dat", sep=" ",col.names=FALSE,row.names = FALSE)
