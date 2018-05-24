## Load dependancies
library(doBy)
library(ggplot2)
library(RColorBrewer)
library(ggrepel)

## Load data
res_unif<-read.csv("../data/20171117_res8unif_alpha.csv",h=T)
res_zipf<-read.csv("../data/20171128_res8zipf_alpha.csv",h=T)

## Filter NA Values
res_zipf<-res_zipf[!is.na(res_zipf$perf_bio),]
res_zipf<-res_zipf[!is.na(res_zipf$perf_sp),]

table(res_unif$alpha)
table(res_zipf$alpha)

## Define color scales
sp_cols=RColorBrewer::brewer.pal(9,"Reds")
bio_cols=RColorBrewer::brewer.pal(9,"Blues")

##### Evolution of Perf/cost vs. Fault tolerance/cost according to alpha values
## Summary of value by alpha

sp_unif<-data.frame(alpha=res_unif$alpha,time=res_unif$time_sp,n_steps=res_unif$n_steps_sp,perf=res_unif$perf_sp,cost=res_unif$cost_sp,ft=res_unif$ft_sp);
bio_unif<-data.frame(alpha=res_unif$alpha,time=res_unif$time_bio,n_steps=res_unif$n_steps_bio,perf=res_unif$perf_bio,cost=res_unif$cost_bio,ft=res_unif$ft_bio);
sp_zipf<-data.frame(alpha=res_zipf$alpha,time=res_zipf$time_sp,n_steps=res_zipf$n_steps_sp,perf=res_zipf$perf_sp,cost=res_zipf$cost_sp,ft=res_zipf$ft_sp);
bio_zipf<-data.frame(alpha=res_zipf$alpha,time=res_zipf$time_bio,n_steps=res_zipf$n_steps_bio,perf=res_zipf$perf_bio,cost=res_zipf$cost_bio,ft=res_zipf$ft_bio);


types<-c(rep("unif sp",nrow(sp_unif)),rep("unif bio",nrow(bio_unif)),rep("zipf sp",nrow(sp_zipf)),rep("zipf bio",nrow(bio_zipf)))

merged_data<-data.frame(alpha=c(sp_unif$alpha,bio_unif$alpha,sp_zipf$alpha,bio_zipf$alpha),
                        type=types,
                        perfcost=c(sp_unif$perf/sp_unif$cost,bio_unif$perf/bio_unif$cost,sp_zipf$perf/sp_zipf$cost,bio_zipf$perf/bio_zipf$cost),
                        ftcost=c(sp_unif$ft/sp_unif$cost,bio_unif$ft/bio_unif$cost,sp_zipf$ft/sp_zipf$cost,bio_zipf$ft/bio_zipf$cost)
)

med_perfcost<-summaryBy(perfcost~alpha+type,data=merged_data,FUN=median)
med_ftcost<-summaryBy(ftcost~alpha+type,data=merged_data,FUN=median)

dt_plot<-data.frame(alphas=med_perfcost$alpha,
                    type=med_perfcost$type,
                    perfcost=med_perfcost[,3],
                    ftcost=med_ftcost[,3])


evo_alpha_plot<-ggplot(dt_plot, aes(x=ftcost, y=perfcost)) +
    geom_point(aes(color=type),shape=19,size = 2) +
    geom_text_repel(aes(x=ftcost, y=perfcost,label=alphas),
                    color =rep(c(bio_cols[7],sp_cols[7],bio_cols[4],sp_cols[4]),length(levels(factor(dt_plot$alphas)))),
                    segment.colour="black",size=5,force = 2) +
   
    scale_color_manual(values=c(bio_cols[7],sp_cols[7],bio_cols[4],sp_cols[4]),
                       name = "Experiment",labels = c("Unif. - Bio","Unif. - SP","Grav. - Bio","Grav. - SP")) +
    ylab("Perf/Cost") + xlab("FT/Cost") +
    geom_path(aes(color=type),size=1) +
    theme_light(base_size = 14)
evo_alpha_plot

ggsave(file="/home/queyroi/cs/MultiSourcesRouting/notes/figs/evo_alpha_plot.eps",plot=evo_alpha_plot)

##### Barplot Perf/Cost/FT according to alphas values
dt_bplot<-data.frame(alpha=factor(c(sp_unif$alpha,bio_unif$alpha,sp_zipf$alpha,bio_zipf$alpha)),
                        type=types,
                        perf=c(sp_unif$perf,bio_unif$perf,sp_zipf$perf,bio_zipf$perf),
                        cost=c(sp_unif$cost,bio_unif$cost,sp_zipf$cost,bio_zipf$cost),
                        ft=c(sp_unif$ft,bio_unif$ft,sp_zipf$ft,bio_zipf$ft)
)

plot_perf<-ggplot(dt_bplot, aes(x=factor(alpha), y=perf,fill=type))+
    geom_boxplot(notch=TRUE, outlier.shape=NA,aes(fill=type),size=0.25) + 
    scale_fill_manual(values=c(bio_cols[7],sp_cols[7],bio_cols[4],sp_cols[4]),name = "Experiment",labels = c("Unif. - Bio","Unif. - SP","Grav. - Bio","Grav. - SP")) +
    ylab("Perf")  + 
    theme_light(base_size = 12) + 
    theme(axis.title.x = element_blank(),panel.grid.major.x = element_blank(),panel.grid.minor.y = element_blank()) 
plot_perf

plot_cost<-ggplot(dt_bplot, aes(x=factor(alpha), y=cost,fill=type))+
    geom_boxplot(notch=TRUE,outlier.shape=NA,aes(fill=type),size=0.25) + 
    scale_fill_manual(values=c(bio_cols[7],sp_cols[7],bio_cols[4],sp_cols[4])) + 
    theme_light(base_size = 12) + 
    theme(axis.title.x = element_blank(),panel.grid.major.x = element_blank(),panel.grid.minor.y = element_blank()) +
    coord_cartesian(ylim = c(0, 0.17)) +
    ylab("Cost") 

plot_ft<-ggplot(dt_bplot, aes(x=factor(alpha), y=ft,fill=type))+
    geom_boxplot(notch=TRUE,outlier.shape=NA,aes(fill=type),size=0.25) + 
    scale_fill_manual(values=c(bio_cols[7],sp_cols[7],bio_cols[4],sp_cols[4])) +
    theme_light(base_size = 12) + 
    ylab("FT") + 
    theme(axis.title.x = element_blank(),panel.grid.major.x = element_blank(),panel.grid.minor.y = element_blank())
    
plot_perfcost<-ggplot(dt_bplot, aes(x=factor(alpha), y=perf/cost,fill=type))+
    geom_boxplot(notch=TRUE,outlier.shape=NA,aes(fill=type),size=0.25) + 
    scale_fill_manual(values=c(bio_cols[7],sp_cols[7],bio_cols[4],sp_cols[4])) + 
    coord_cartesian(ylim = c(2.5, 25)) +
    theme_light(base_size = 12) + 
    ylab("Perf/Cost")  + 
    theme(axis.title.x = element_blank(),panel.grid.major.x = element_blank(),panel.grid.minor.y = element_blank()) 

plot_ftcost<-ggplot(dt_bplot, aes(x=factor(alpha), y=ft/cost,fill=type))+
    geom_boxplot(notch=TRUE,outlier.shape=NA,aes(fill=type),size=0.25) + 
    scale_fill_manual(values=c(bio_cols[7],sp_cols[7],bio_cols[4],sp_cols[4])) + 
    coord_cartesian(ylim = c(5, 25)) +
    theme_light(base_size = 12) + 
    ylab("FT/Cost")  + 
    theme(axis.title.x = element_blank(),panel.grid.major.x = element_blank(),panel.grid.minor.y = element_blank())
    
dt_diff<-data.frame(alpha=c(res_unif$alpha,res_zipf$alpha),
                    flow=c(rep("unif",nrow(res_unif)),rep("zipf",nrow(res_zipf))),
                    diff=c(res_unif$diffset,res_zipf$diffset)) 

diff_cols=RColorBrewer::brewer.pal(9,"Greys")
plot_diff<-ggplot(dt_diff,aes(x=factor(alpha),y=diff,fill=flow)) +
    geom_boxplot(notch=TRUE,outlier.shape=NA,aes(fill=flow),size=0.25)  + 
    scale_fill_manual(values=c(diff_cols[7],diff_cols[4]),name="Type",labels=c("Unif.","Grav.")) +
    theme_light(base_size = 12) + 
    ylab("Similarity")  + 
    theme(axis.title.x = element_blank(),panel.grid.major.x = element_blank(),panel.grid.minor.y = element_blank())
    

library(gridExtra)
library(grid)
library(lattice)

#grid.arrange(plot_perf,plot_perfcost,plot_ft,plot_ftcost,plot_cost,plot_diff,bottom="alpha")
grid_arrange_shared_legend <- function(..., ncol = length(list(...)), nrow = 1, position = c("bottom", "right")) {
    plots <- list(...)
    position <- match.arg(position)
    g <- ggplotGrob(plots[[1]] + 
                        theme(legend.position = position))$grobs
    legend <- g[[which(sapply(g, function(x) x$name) == "guide-box")]]
    lheight <- sum(legend$height)
    lwidth <- sum(legend$width)
    gl <- lapply(plots, function(x) x +
                     theme(legend.position = "none"))
    gl[[6]] <- gl[[6]] + theme(legend.position = "right")
    gl <- c(gl, ncol = ncol, nrow = nrow)
    
    combined <- switch(position,
                       "bottom" = arrangeGrob(do.call(arrangeGrob, gl), 
                                              legend,ncol = 1,
                                              heights = unit.c(unit(1, "npc") - lheight, lheight)),
                       "right" = arrangeGrob(do.call(arrangeGrob, gl),
                                             legend, ncol = 2,
                                             widths = unit.c(unit(1, "npc") - lwidth, lwidth)))
    
    grid.newpage()
    grid.draw(combined)
    
    # return gtable invisibly
    invisible(combined)
}
grid_arrange_shared_legend(plot_perf,plot_perfcost,plot_ft,plot_ftcost,plot_cost,plot_diff, ncol = 2, nrow = 3)

barplot_final<-grid_arrange_shared_legend(plot_perf,plot_perfcost,plot_ft,plot_ftcost,plot_cost,plot_diff, ncol = 2, nrow = 3)
ggsave(file="/home/queyroi/cs/MultiSourcesRouting/notes/figs/barplot_alpha.eps",plot=barplot_final)
