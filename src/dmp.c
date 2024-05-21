#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include <linux/bio.h>
#include <linux/device-mapper.h>
#include <linux/kobject.h>


struct operation_statistics {
    unsigned long write_reqs_count;
    unsigned long read_reqs_count;
    unsigned long write_sum_block_size;
    unsigned long read_sum_block_size;
};

struct dmp_target {
    struct dm_dev *dev;
    struct operation_statistics stat;
    struct kobject stat_kobj;
};

static struct attribute volumes_attribute = {
    .name = "volumes",
    .mode = 0444,
};

static ssize_t show_stat(struct kobject *kobj, struct attribute *attr, char *buf) {
    struct dmp_target *dt = container_of(kobj, struct dmp_target, stat_kobj);
    unsigned long avg_reqs_count = dt->stat.read_reqs_count + dt->stat.write_reqs_count;
    return sprintf(buf, "read:\n"
                        "\treqs: %lu\n"
                        "\tavg size: %lu\n"
                        "write:\n"
                        "\treqs: %lu\n"
                        "\tavg size: %lu\n"
                        "total:\n"
                        "\treqs: %lu\n"
                        "\tavg size: %lu\n",
                        dt->stat.read_reqs_count, dt->stat.read_sum_block_size / dt->stat.read_reqs_count,
                        dt->stat.write_reqs_count, dt->stat.write_sum_block_size / dt->stat.write_reqs_count,
                        avg_reqs_count, (dt->stat.read_sum_block_size + dt->stat.write_sum_block_size) / avg_reqs_count);
}

struct sysfs_ops volumes_sysfs_ops = {
    .show = show_stat,
};

static const struct kobj_type stat_ktype = {
    .sysfs_ops = &volumes_sysfs_ops,
};

static int proxy_target_map(struct dm_target *ti, struct bio *bio)
{
    struct dmp_target *dt = (struct dmp_target *) ti->private;
    unsigned int block_size = bio->bi_iter.bi_size;

    switch (bio_op(bio)) {
	case REQ_OP_READ:
        dt->stat.read_reqs_count += 1;
        dt->stat.read_sum_block_size += block_size;
		break;
	case REQ_OP_WRITE:
        dt->stat.write_reqs_count += 1;
        dt->stat.write_sum_block_size += block_size;
		break;
	default:
		return DM_MAPIO_KILL;
	}

	bio_endio(bio);

	return DM_MAPIO_SUBMITTED;
}

static int proxy_target_ctr(struct dm_target *ti, unsigned int argc, char **argv)
{
    if (argc != 1) {
        printk(KERN_CRIT "\nproxy_target_ctr: invalid argument count\n");
        ti->error = "proxy_target_ctr: Invalid argument count";
        return -EINVAL;
    }

    struct dmp_target *dt = kmalloc(sizeof(struct dmp_target), GFP_KERNEL);

    if(dt == NULL)
    {
        printk(KERN_CRIT "\nproxy_target_ctr: cannot allocate linear context for dmp_target\n");
        ti->error = "dmp_target: Cannot allocate linear context";
        return -ENOMEM;
    }       

    if (dm_get_device(ti, argv[0], dm_table_get_mode(ti->table), &dt->dev)) {
        ti->error = "dmp_target: Device lookup failed";
        kfree(dt);         
        return -EINVAL;
    }

    dt->stat.write_reqs_count = 0;
    dt->stat.read_reqs_count = 0;
    dt->stat.write_sum_block_size = 0;
    dt->stat.read_sum_block_size = 0;

    int rc = kobject_init_and_add(&dt->stat_kobj, &stat_ktype, &THIS_MODULE->mkobj.kobj, "stat");

    if(rc)
    {
        printk(KERN_CRIT "\nproxy_target_ctr: dmp statistics kobject is null\n");
        kfree(dt);         
        return rc;
    }

    rc = sysfs_create_file(&dt->stat_kobj, &volumes_attribute);

    if(rc) {
        printk(KERN_CRIT "\nerror in creating sysfs volumes file\n");
        kobject_put(&dt->stat_kobj);
        kfree(dt);
        return rc;
    }

    ti->private = dt;
                
    return 0;
}

static void proxy_target_dtr(struct dm_target *ti)
{
    struct dmp_target *dt = (struct dmp_target *) ti->private;
    kobject_put(&dt->stat_kobj);
    dm_put_device(ti, dt->dev);
    kfree(dt);
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
        printk(KERN_CRIT "\nerror in registering target\n");
    return 0;
}

static void cleanup_proxy_target(void)
{
    dm_unregister_target(&proxy_target);
}

module_init(init_proxy_target);
module_exit(cleanup_proxy_target);
MODULE_LICENSE("GPL");