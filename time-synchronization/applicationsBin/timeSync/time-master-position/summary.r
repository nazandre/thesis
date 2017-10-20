#! /bin/Rscript

#verbose <- TRUE
verbose <- FALSE

dir <- commandArgs(TRUE)[1]

printIfVerbose <- function(v) {
  if (verbose) {
     print(v)
  }
}

versions <- c("extremities", "center", "centroid", "betweennessCenter")
for (i in 1:length(versions)){
    # open input files
    file <- paste(dir,"/error.",versions[i],sep="")
    cat("file: ", file, "\n")
    data <- read.table(paste(file))
    
    colnames(data) <- c("time", "range", "sd", "madev", "madiff")
    printIfVerbose(data)

    #data <- data[data$time > 5*60*1000*1000]
    data <- subset(data,data$time > 5*60*1000*1000)
    avgRange=mean(data$range)
    cat("Avg range:",avgRange,"\n")

    maxRange=max(data$range)
    cat("Max range:",maxRange,"\n")

    sdRange=sd(data$range)
    cat("sd range:",sdRange,"\n")

    avgMadev=mean(data$madev)
    cat("Avg MADev:",avgMadev,"\n")
    
    avgMadiff=mean(data$madiff)
    cat("Avg MADiff:",avgMadiff,"\n")
    
    avgSd=sqrt(sum(data$sd*data$sd)/length(data$sd))
    cat("Avg SD:",avgSd,"\n")
    
}

