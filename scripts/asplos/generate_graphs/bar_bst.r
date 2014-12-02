library("ggplot2")
library("scales")
library("grid")
library("gridExtra")
args <- commandArgs()
mydata=read.csv(args[5],sep=" ")

g_legend <- function(p){
   tmp <- ggplot_gtable(ggplot_build(p))
   leg <- which(sapply(tmp$grobs, function(x) x$name) == "guide-box")
   legend <- tmp$grobs[[leg]]
   return(legend)
}
mydata$structure <- factor(mydata$structure, levels =c("async-ext","async-int","bronson","drachsler","ellen","howley","natarajan"))
mydata$machine <- factor(mydata$machine, levels=c("Opteron","Xeon20","Xeon40","Tilera","T4-4"))

high=subset(mydata,experiment=="high")
low=subset(mydata,experiment=="low")
theme_set(theme_bw())
 theme_update(plot.margin = unit(c(0,0,0,0), "cm"))
 mvh=max(high$throughput)*1.4
 mvl=max(low$throughput)*1.4
p1=ggplot(data=high, aes(x=machine, y=throughput, fill=structure)) +xlab("")+ylab("Throughput (Mops/s)") +geom_bar(stat="identity", colour="black",position=position_dodge())+ggtitle("High contention")+geom_text(angle = 90, aes(label = paste(sprintf("%.1f", scalability), "", sep=""), y =throughput+mvh/9), size = 3.7, position = position_dodge(width=0.9)) +scale_fill_brewer(palette="OrRd") + scale_x_discrete(expand = c(0, 0)) + scale_y_continuous(expand = c(0, 0),limits= c(0,mvh)) + theme(legend.key.size = unit(0.45, "cm"))+theme(legend.text = element_text(size = 13))+theme(legend.title = element_text(size = 13))+theme(axis.text.y = element_text(size = )) + theme(axis.text.x = element_text(size = 12)) + opts( panel.grid.major = theme_blank(),panel.grid.minor = theme_blank())  + theme(panel.border= element_rect(colour="black"))
p2=ggplot(data=low, aes(x=machine, y=throughput, fill=structure))+ggtitle("Low contention") +xlab("")+ylab("Throughput (Mops/s)") +geom_bar(stat="identity", colour="black",position=position_dodge())+geom_text(angle = 90, aes(label = paste(sprintf("%.1f", scalability), "", sep=""), y =throughput+mvl/9), size = 3.7, position = position_dodge(width=0.9)) +scale_fill_brewer(palette="OrRd") + scale_x_discrete(expand = c(0, 0)) + scale_y_continuous(expand = c(0, 0),limits= c(0,mvl))+ theme(axis.text.y = element_text(size = )) + theme(axis.text.x = element_text(size = 12)) +  opts( panel.grid.major = theme_blank(),panel.grid.minor = theme_blank()) + theme(panel.border= element_rect(colour="black"))


leg<-g_legend(p1)
dev.off()
pdf(file=args[7],height=2,width=14)
grid.arrange(p1+theme(legend.position="none"),p2+theme(legend.position="none"),leg,ncol=3,widths=c(31/70,31/70,8/70))
#print(p1 + scale_x_discrete(expand = c(0, 0)) + scale_y_discrete(expand = c(0, 0)))
dev.off()
