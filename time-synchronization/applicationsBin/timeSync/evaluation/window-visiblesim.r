# execute script: Rscript file.r


dir <- "results/fidelity/window/"
window <- c(2,3,5,10,20,30)

windowSummary <- matrix(nrow=length(window),ncol=6)


for (i in 1:length(window)){
    file <- paste(dir, "mrtp.window-", window[i], sep="")
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
    #print(test)    
    error <- error[5:length(test)]

    # Proba
    print(summary(abs(error)))
    print(sd(error))
   
    absError <- error
    #absError <- abs(error)
    
    windowSummary[i,1] = as.numeric(window[i])
    windowSummary[i,2] = mean(absError)
    windowSummary[i,3] = min(absError)
    windowSummary[i,4] = max(absError)
    windowSummary[i,5] = sd(absError)
    windowSummary[i,6] = quantile(sort(absError),c(.99))
   
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
    write.table(da3,  quote = FALSE, file = paste(dir,window[i],"w.dat",sep=""), sep=" ",col.names=FALSE,row.names = FALSE)
}
   windowSummary <- windowSummary[sort.list(windowSummary[,1]), ]
   print("Summary:")
   print(windowSummary)
   write.table(windowSummary, quote = FALSE, file = paste(dir,"summary.dat",sep=""), sep=" ", col.names=FALSE, row.names = FALSE)
