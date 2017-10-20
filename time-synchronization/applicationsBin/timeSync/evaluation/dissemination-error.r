dir <- "distance"

files <- list.files(path=dir, pattern = "[0-9]+.dat", full.names = TRUE, recursive = TRUE)

mySummary <- matrix(nrow=length(files),ncol=6)

#cat("#files: ", length(files), "\n")

for (i in 1:length(files)){

#    cat("file: ", files[i], "\n")
    clockData <- read.table(files[i])
    
    # 1: id
    # 2: simTime
    # 3: realGlobalTime (master global time) 
    # 4: estimatedGlobalTime (msg)
    # 5: localTime
    # 6: local global time
    # 7: distance
    level <- as.numeric(clockData[1,7])
#    cat("level: ", level, "\n")

    realGlobal <- clockData[,3]/1000
    estimatedGlobal <- clockData[,4]/1000
    error <- (estimatedGlobal - realGlobal)

    filter <- 1000*60*60
    test <- estimatedGlobal[ estimatedGlobal < filter]
    #print(test)    
    error <- error[5:length(test)]

    #error <- error[10:length(error)]
    
#   print(summary(abs(error)))
#   print(mean(error))

   absError <- error
   #abs(error)

   mySummary[i,1] = as.numeric(level)
   mySummary[i,2] = mean(absError)
   mySummary[i,3] = min(absError)
   mySummary[i,4] = max(absError)
   mySummary[i,5] = sd(absError)
   mySummary[i,6] = quantile(sort(absError),c(.99))
  
   a <- table(error)
   f <- as.data.frame(a)
   x <- c(sort(unique(error)))
   y <-f$Freq/length(error)*100
   
   total <- sum(y)
#   cat("total: ", total, "\n")
   
   dataToExport <- data.frame(x,y)
   da2 <-as.matrix(dataToExport)
   da3 <- matrix(da2, ncol=ncol(da2),dimnames = NULL)
   names(da3) <- NULL

   write.table(da3,  quote = FALSE, file = paste(dir, "/", level, "-dissemination-error-density.dat",sep=""), sep=" ",col.names=FALSE,row.names = FALSE)
}

mySummary <- mySummary[sort.list(mySummary[,1]), ]

print("Summary:")
print("hop, mean, min, max, sd, 99th quantile")
print(mySummary)

write.table(mySummary, quote = FALSE, file = paste(dir, "/dissemination-error-summary.dat",sep=""), sep=" ", col.names=FALSE, row.names = FALSE)
