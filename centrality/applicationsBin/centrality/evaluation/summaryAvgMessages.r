#! /bin/Rscript

library(dplyr)

#verbose <- TRUE
verbose <- FALSE

printIfVerbose <- function(v) {
  if (verbose) {
     print(v)
  }
}

exportToFile <- function(v,t) {
   write.table(v, quote = FALSE, file = paste(t,".dat",sep=""), sep=" ", col.names=FALSE, row.names = FALSE)
}

# open input files
file <- commandArgs(TRUE)[1]
data <- read.table(file)

# format
colnames(data) <- c("size", "id", "messages", "queue", "motions")
printIfVerbose(data)

# total msg + sd per size
messages <- summarise(group_by(data, size), mean=mean(messages), sd=sd(messages), count=n())
printIfVerbose(messages)
exportToFile(messages,"avgMessages2")