#! /bin/Rscript

library(dplyr)

#verbose <- TRUE
verbose <- FALSE

computeError <- function(real,approx) {
  error <- abs(real-approx)/real
  return(error)
}

printIfVerbose <- function(v) {
  if (verbose) {
     print(v)
  }
}

exportToFile <- function(v,t) {
   write.table(v, quote = FALSE, file = paste(t,".dat",sep=""), sep=" ", col.names=FALSE, row.names = FALSE)
}

usToS <- function(t) {
  return(t/1000/1000)
}

# open input files
file <- commandArgs(TRUE)[1]
data <- read.table(file)


# format
#$size $links $diameter $theoricEcc $theoricFar $time $avgMessages $maxQueueIn $maxQueueOut $electedEcc $electedFarness
colnames(data) <- c("size", "links", "diameter", "radius", "minFarness", "time", "messages", "queue", "eccentricity", "farness", "avgMessages", "sdMessages", "memory")
printIfVerbose(data)

data$time <- usToS(data$time)

# total msg + sd per size
messages <- summarise(group_by(data, size), mean=mean(messages), sd=sd(messages), count=n())
printIfVerbose(messages)
exportToFile(messages,"messages")

#https://stats.stackexchange.com/questions/25848/how-to-sum-a-standard-deviation
avgMessages <- summarise(group_by(data, size), mean=mean(avgMessages), sd=sum(sdMessages*sdMessages), count=n())
avgMessages$sd <- sqrt(avgMessages$sd/avgMessages$count)
#avgMessages$sd <- avgMessages$sd/avgMessages$count
printIfVerbose(avgMessages)
exportToFile(avgMessages,"avgMessages")

# avg error + sd per size
errorEcc <- summarise(group_by(data, size), mean=mean(computeError(radius,eccentricity)), sd=sd(computeError(radius,eccentricity)),count=n())
printIfVerbose(errorEcc)
exportToFile(errorEcc,"errorEcc")

# ecc + farness
errorFar <- summarise(group_by(data, size), mean=mean(computeError(minFarness,farness)), sd=sd(computeError(minFarness,farness)),count=n())
printIfVerbose(errorFar)
exportToFile(errorFar,"errorFar")

# time (avg+sd) per diameter
time <- summarise(group_by(data, diameter), mean=mean(time), sd=sd(time),count=n())
printIfVerbose(time)
exportToFile(time,"time")

# queue per size
queue <- summarise(group_by(data, size), max=max(queue),count=n())
printIfVerbose(queue)
exportToFile(queue,"queue")

# maximum per node memory usage to store alg-specific variables + messages
memory <- summarise(group_by(data, size), max=max(memory),count=n())
msgSize = 17 + 2 # data + handler (ignore crc because can be computed on fly)
memory$max <- memory$max + (msgSize)*queue$max
printIfVerbose(memory)
exportToFile(memory,"memory")