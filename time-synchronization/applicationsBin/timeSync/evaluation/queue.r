file <- commandArgs(TRUE)[1]
data <- read.table(file)
queueIndex <- 3
queue <- data[,queueIndex]
maxQueue <- max(queue)
cat(maxQueue)
