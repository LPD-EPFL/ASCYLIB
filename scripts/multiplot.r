library("quantmod")
library("ggplot2")
library("RColorBrewer")
library("reshape2")
library("plyr")
library("scales")
library("reshape")
args <- commandArgs()

#throughput
mydata = read.csv(args[5], check.names="FALSE",sep=",",header=T)
nc=ncol(mydata)
mydata<-mydata[,1:nc-1]
colnames(mydata)[1] <- "updates"
dat<-melt(mydata,id=c("updates"),variable_name="S")
dat$experiment<-apply(dat,1,function(row) "Throughput")
dat <- transform(dat, value = value /100)
dat$val2=dat$value

#scalability
mydata2 = read.csv(args[6], check.names="FALSE",sep=",",header=T)
nc=ncol(mydata2)
mydata2<-mydata2[,1:nc-1]
colnames(mydata2)[1] <- "updates"
dat2<-melt(mydata2,id=c("updates"),variable_name="S")
dat2$experiment<-apply(dat2,1,function(row) "Scalability")
dat2$val2=dat2$value

#get latency
mydata3 = read.csv(args[7], check.names="FALSE",sep=",",header=T)
nc=ncol(mydata3)
mydata3<-mydata3[,1:nc-1]
colnames(mydata3)[1] <- "updates"
dat3<-melt(mydata3,id=c("updates"),variable_name="S")
dat3$experiment<-apply(dat3,1,function(row) "Get Lat.")
dat3$val2=1/dat3$value

#put latency
mydata4 = read.csv(args[8], check.names="FALSE",sep=",",header=T)
nc=ncol(mydata4)
mydata4<-mydata4[,1:nc-1]
colnames(mydata4)[1] <- "updates"
dat4<-melt(mydata4,id=c("updates"),variable_name="S")
dat4$experiment<-apply(dat4,1,function(row) "Put Lat.")
dat4$val2=1/dat4$value

#rem latency
mydata5 = read.csv(args[9], check.names="FALSE",sep=",",header=T)
nc=ncol(mydata5)
mydata5<-mydata5[,1:nc-1]
colnames(mydata5)[1] <- "updates"
dat5<-melt(mydata5,id=c("updates"),variable_name="S")
dat5$experiment<-apply(dat5,1,function(row) "Rem Lat.")
dat5$val2=1/dat5$value

total <- rbind(dat5, dat4, dat3, dat2, dat)

myBreaks=c(0, 0.33, 0.50, 0.75, 0.85, 0.97, 1.03, 1.1, 1.3, 1.5, 2, Inf)
col = brewer.pal(11,"RdYlGn")
total$rat3=findInterval(total$val2, myBreaks, all.inside = TRUE)
print(args[10])
print(total)
label_text <- rollapply(round(myBreaks, 2), width = 2, by = 1, FUN = function(i) paste(i, collapse = " : "))

pdf(args[10], height=5, width=15)
myplot <- ggplot(total, aes(factor(S), factor(updates), fill = factor(rat3,levels=c("1","2","3","4","5","6","7","8","9","10","11")))) +  
  geom_tile(colour = "white") +
#geom_text(size=3,aes(fill=factor(total$rat3,levels=c("1","2","3","4","5","6","7","8","9","10","11")), label = round(total$val2,2))) +
  facet_grid(.~experiment) +
  scale_fill_manual(values = col, name="Ratios", labels = label_text, drop=FALSE) +
  labs(title = args[11]) +
  xlab("cores") +
  ylab("update ratio") 

print(myplot)
dev.off()

