#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include <linux/bio.h>
#include <linux/device-mapper.h>


struct operation_statistics {
    unsigned long write_reqs_count;
    unsigned long read_reqs_count;
    unsigned long avg_reqs_count;           //delete
    unsigned long write_sum_block_size;
    unsigned long read_sum_block_size;
    unsigned int write_avg_block_size;      //delete
    unsigned int read_avg_block_size;       //delete
    unsigned int avg_block_size;            //delete
};

struct dmp_target {
    struct dm_dev *dev;
    struct operation_statistics stat;
};

static int proxy_target_map(struct dm_target *ti, struct bio *bio)
{
    struct dmp_target *dt = (struct dmp_target *) ti->private;
    unsigned int block_size = bio->bi_iter.bi_size;
    dt->stat.avg_reqs_count += 1;
    dt->stat.avg_block_size = (dt->stat.read_sum_block_size + dt->stat.write_sum_block_size) / dt->stat.avg_reqs_count;
    switch (bio_op(bio)) {
	case REQ_OP_READ:
        dt->stat.read_reqs_count += 1;
        dt->stat.read_sum_block_size += block_size;
        dt->stat.read_avg_block_size = dt->stat.read_sum_block_size / dt->stat.read_reqs_count;
		break;
	case REQ_OP_WRITE:
        dt->stat.write_reqs_count += 1;
        dt->stat.write_sum_block_size += block_size;
        dt->stat.write_avg_block_size = dt->stat.write_sum_block_size / dt->stat.write_reqs_count;
		break;
	default:
		return DM_MAPIO_KILL;
	}

    printk(KERN_INFO "%u %u %lu", dt->stat.read_avg_block_size, dt->stat.write_avg_block_size, dt->stat.avg_reqs_count);
	bio_endio(bio);

	return DM_MAPIO_SUBMITTED;
}

static int proxy_target_ctr(struct dm_target *ti, unsigned int argc, char **argv)
{
    struct dmp_target *dt;

    printk(KERN_CRIT "\n >>in function proxy_target_ctr \n");

    if (argc != 1) {
            printk(KERN_CRIT "\n Invalid argument count\n");
            ti->error = "Invalid argument count";
            return -EINVAL;
    }

    dt = kmalloc(sizeof(struct dmp_target), GFP_KERNEL);

    if(dt==NULL)
    {
        printk(KERN_CRIT "\n DMP target is null\n");
        ti->error = "dmp_target: Cannot allocate linear context";
        return -ENOMEM;
    }       
        
    if (dm_get_device(ti, argv[0], dm_table_get_mode(ti->table), &dt->dev)) {
        ti->error = "dmp_target: Device lookup failed";
        goto bad;
    }

    dt->stat.write_reqs_count = 0;
    dt->stat.read_reqs_count = 0;
    dt->stat.avg_reqs_count = 0;
    dt->stat.write_sum_block_size = 0;
    dt->stat.read_sum_block_size = 0;
    dt->stat.write_avg_block_size = 0;
    dt->stat.read_avg_block_size = 0;
    dt->stat.avg_block_size = 0;
    ti->private = dt;

    printk(KERN_CRIT "\n>>out function proxy_target_ctr \n");                      
    return 0;

    bad:
        kfree(dt);
        printk(KERN_CRIT "\n>>out function proxy_target_ctr with error \n");           
        return -EINVAL;
}

static void proxy_target_dtr(struct dm_target *ti)
{
    struct dmp_target *dt = (struct dmp_target *) ti->private;
    printk(KERN_CRIT "\n<<in function proxy_target_dtr \n");        
    dm_put_device(ti, dt->dev);
    kfree(dt);
    printk(KERN_CRIT "\n>>out function proxy_target_dtr \n");  
}

static struct target_type proxy_target = {
    .name = "dmp",
    .version = {1,0,0},
    .module = THIS_MODULE,
    .ctr = proxy_target_ctr,
    .dtr = proxy_target_dtr,
    .map = proxy_target_map,
};

static int init_proxy_target(void)
{
    int rc = dm_register_target(&proxy_target);
    if(rc < 0)
        printk(KERN_CRIT "\n Error in registering target \n");
    return 0;
}

static void cleanup_proxy_target(void)
{
    dm_unregister_target(&proxy_target);
}

module_init(init_proxy_target);
module_exit(cleanup_proxy_target);
MODULE_LICENSE("GPL");