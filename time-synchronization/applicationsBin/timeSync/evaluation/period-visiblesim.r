# execute script: period.r

dir <- "results/fidelity/period/"
period <- c(2,5,10,20,30,60)

periodSummary <- matrix(nrow=length(period),ncol=6)

#for (i in 5:5){
for (i in 1:length(period)){
    file <- paste(dir, "mrtp.period-", period[i], sep="")
    cat("file: ", file, "\n")
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
error <- (estimated - global)

      
    filter <- 1000*60*60
    test <- estimated[ estimated < filter ]
    error <- error[5:length(test)]

    # Proba
    print(summary(abs(error)))
    print("quantile")
    print(quantile(sort(abs(error)),c(.99)))
    print("sd:")
    print(sd(error))
    
    absError <- error
# <- abs(error)

    periodSummary[i,1] = as.numeric(period[i])
    periodSummary[i,2] = mean(absError)
    periodSummary[i,3] = min(absError)
    periodSummary[i,4] = max(absError)
    periodSummary[i,5] = sd(absError)
    periodSummary[i,6] = quantile(sort(absError),c(.99))
    
    a <- table(error)
    f <- as.data.frame(a)
    x <- c(sort(unique(error)))
    y <-f$Freq/length(error)*100

    total <- sum(y)
    cat("total: ", total, "\n")

    dataToExport <- data.frame(x,y)
    da2 <-as.matrix(dataToExport)
    da3 <- matrix(da2, ncol=ncol(da2),dimnames = NULL)
    names(da3) <- NULL
    write.table(da3,  quote = FALSE, file = paste(dir,period[i],"sec.dat",sep=""), sep=" ",col.names=FALSE,row.names = FALSE)
}
   periodSummary <- periodSummary[sort.list(periodSummary[,1]), ]
   print("Summary:")
   print(periodSummary)
   write.table(periodSummary, quote = FALSE, file = paste(dir,"summary.dat",sep=""), sep=" ", col.names=FALSE, row.names = FALSE)
