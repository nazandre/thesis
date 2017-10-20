
#dir <- "exp-new-new/exp14-quality-estimation4-offset/"
#dir <- "exp-new-new/exp15-quality-estimation4-ptp/"
#dir <- "exp-new-new/exp16-quality-estimation20-ptp/"
#dir <- "exp-new-new/exp17-quality-estimation20-ptp/"

dir <- "quality-data/"

files <- list.files(path=dir, pattern = "[0-9]+.dat", full.names = TRUE, recursive = TRUE)

mySummary <- matrix(nrow=length(files),ncol=6)

#cat("#files: ", length(files), "\n")

for (i in 1:length(files)){
    #cat("file: ", files[i], "\n")
    clockData <- read.table(files[i])
    level <- as.numeric(clockData[1,6])
    #cat("level: ", level, "\n")
    global <- clockData[,4]
    estimated <- clockData[,2]
    error <- (estimated - global)

    filter <- 1000*60*60
    global <- global[global < filter]
    error <- error[5:length(global)]
    
# Proba
   #print(summary(abs(error)))
   #print(sd(error))

   absError <- error
#<- abs(error)

   mySummary[i,1] = as.numeric(level)
   mySummary[i,2] = mean(absError)
   mySummary[i,3] = min(absError)
   mySummary[i,4] = max(absError)
   mySummary[i,5] = sd(absError)
   mySummary[i,6] = quantile(sort(abs(absError)),c(.99))

 cat("quantile: ", quantile(sort(absError),c(.95)),"\n")
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

   write.table(da3,  quote = FALSE, file = paste(dir, "/", level, "-density.dat",sep=""), sep=" ",col.names=FALSE,row.names = FALSE)
}

mySummary <- mySummary[sort.list(mySummary[,1]), ]
print("Summary:")
print(mySummary)
write.table(mySummary, quote = FALSE, file = paste(dir, "/summary.dat",sep=""), sep=" ", col.names=FALSE, row.names = FALSE)
