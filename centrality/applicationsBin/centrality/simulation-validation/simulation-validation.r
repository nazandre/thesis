file <- commandArgs(TRUE)[1]
data <- read.table(file)

value <- floor(data[,1]/1000)
#value <- value - 1000
m <- mean(value)
s <- sd(value)
cat(m,s,"\n")
