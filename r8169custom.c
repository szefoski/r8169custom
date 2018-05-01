#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/pci.h>
#include <linux/etherdevice.h>
#include <linux/pci-aspm.h>
#include <linux/if_link.h>

MODULE_LICENSE("Dual BSD/GPL");
#define MODULENAME "r8169custom"

enum cfg_version {
	RTL_CFG_0 = 0x00,
	RTL_CFG_1,
	RTL_CFG_2
};

static const struct pci_device_id rtl8169_pci_tbl[] = {
	{ PCI_DEVICE(PCI_VENDOR_ID_REALTEK,	0x8168), 0, 0, RTL_CFG_1 },
	{0,},
};

MODULE_DEVICE_TABLE(pci, rtl8169_pci_tbl);

struct rtl_private
{
	struct pci_dev *pci_dev;
	void *mmio_addr;
	unsigned long regs_len;
};

static struct net_device *rtl_dev;


static int rtl_open(struct net_device *dev)
{
		printk(KERN_ALERT "rtl_open\n");
	return 0;
}

static int rtl8169_close(struct net_device *dev)
{
	return 0;
}

static netdev_tx_t rtl8169_start_xmit(struct sk_buff *skb,
				      struct net_device *dev)
{
	return NETDEV_TX_OK;
}

static int rtl8169_xmit_frags(struct rtl8169_private *tp, struct sk_buff *skb,
			      u32 *opts)
{
	return 0;
}

static int rtl8169_change_mtu(struct net_device *dev, int new_mtu)
{
	return 0;
}

static netdev_features_t rtl8169_fix_features(struct net_device *dev,
	netdev_features_t features)
{
	return features;
}

static int rtl8169_set_features(struct net_device *dev,
				netdev_features_t features)
{
	return 0;
}

static int rtl_set_mac_address(struct net_device *dev, void *p)
{
	return 0;
}

static int rtl8169_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	return 0;
}

static void rtl_set_rx_mode(struct net_device *dev)
{
}

static void rtl8169_tx_timeout(struct net_device *dev)
{
}

static void rtl8169_get_stats64(struct net_device *dev, struct rtnl_link_stats64 *stats)
{
}

#ifdef CONFIG_NET_POLL_CONTROLLER
static void rtl8169_netpoll(struct net_device *dev)
{
}
#endif

static const struct net_device_ops rtl_netdev_ops = {
	.ndo_open		= rtl_open,
	.ndo_stop		= rtl8169_close,
	.ndo_get_stats64	= rtl8169_get_stats64,
	.ndo_start_xmit		= rtl8169_start_xmit,
	.ndo_tx_timeout		= rtl8169_tx_timeout,
	.ndo_validate_addr	= eth_validate_addr,
	.ndo_change_mtu		= rtl8169_change_mtu,
	.ndo_fix_features	= rtl8169_fix_features,
	.ndo_set_features	= rtl8169_set_features,
	.ndo_set_mac_address	= rtl_set_mac_address,
	.ndo_do_ioctl		= rtl8169_ioctl,
	.ndo_set_rx_mode	= rtl_set_rx_mode,
#ifdef CONFIG_NET_POLL_CONTROLLER
	.ndo_poll_controller	= rtl8169_netpoll,
#endif

};

static int rtl_init(struct pci_dev *pdev, struct net_device **dev_out)
{
	struct net_device *dev;
	struct rtl_private *tp;
	int rc;

	dev = alloc_etherdev(sizeof(struct rtl_private));
	if (!dev) {
		printk(KERN_ALERT "Could not alloc etherdev\n");
		return -1;
	}
	
	SET_NETDEV_DEV(dev, &pdev->dev);	
	dev->netdev_ops = &rtl_netdev_ops;
	rc = register_netdev(dev);
	if (rc < 0)
		printk(KERN_ALERT "Error Error Error !!!!\n");
	
	tp = netdev_priv(dev);
	tp->pci_dev = pdev;
	*dev_out = dev;

	return 0;
}

static int rtl_init_one(struct pci_dev *pdev, const struct pci_device_id *ent)
{
	int rc = 0;
	unsigned long mmio_start, mmio_end, mmio_len, mmio_flags;
	struct net_device *dev = NULL;
	int region = 2;

	printk(KERN_ALERT "Vendor: 0x%04x\n", pdev->vendor);
	printk(KERN_ALERT "Device: 0x%04x\n", pdev->device);

	/* disable ASPM completely as that cause random device stop working
	 * problems as well as full system hangs for some PCIe devices users */
	pci_disable_link_state(pdev, PCIE_LINK_STATE_L0S | PCIE_LINK_STATE_L1 |
				     PCIE_LINK_STATE_CLKPM);

	rc = pci_enable_device(pdev);
	if (rc < 0) {
		printk(KERN_ALERT "HW can not be enable\n");
	}
	else
	{
		printk(KERN_ALERT "HW enable\n");
	}

	printk(KERN_ALERT "MSI vectors %d\n", pci_msi_vec_count(pdev));

	if (pci_set_mwi(pdev) < 0)
	{
	}	

	rtl_init(pdev, &dev);
	rtl_dev = dev;

	if (!pci_is_pcie(pdev))
		printk(KERN_ALERT "not PCI Express\n");
	else
		printk(KERN_ALERT "PCI Express\n");

	mmio_start = pci_resource_start(pdev, region);
	mmio_end = pci_resource_end(pdev, region);
	mmio_len = pci_resource_len(pdev, region);
	mmio_flags = pci_resource_flags(pdev, region);

	printk(KERN_ALERT "PCI start res %ld\n", mmio_start);
	printk(KERN_ALERT "PCI end res %ld\n", mmio_end);
	printk(KERN_ALERT "PCI len res %ld\n", mmio_len);
	printk(KERN_ALERT "PCI end flags %ld\n", mmio_flags);

	if (!(pci_resource_flags(pdev, region) & IORESOURCE_MEM)) {
		printk(KERN_ALERT "region not MMI/O region\n");
	}

	if (pci_request_regions(pdev, MODULENAME))
	{
		printk("Could not get PCI region\n");
	}

	printk(KERN_ALERT "IRQ %d\n", pdev->irq);
	return 0;
}

static void rtl_remove_one(struct pci_dev *pdev)
{
	struct rtl_private *tp;
	printk(KERN_ALERT "is rtl_dev null: %pf\n", rtl_dev);
	if (rtl_dev != NULL)
	{
		tp = netdev_priv(rtl_dev);
		printk(KERN_ALERT "is tp null: %pf\n", tp);
		printk(KERN_ALERT "is tp->pci_dev null: %pf\n", tp->pci_dev);
		pci_release_regions(tp->pci_dev);
	}

}

static void rtl_shutdown(struct pci_dev *pdev)
{
}



static struct pci_driver rtl8169_pci_driver = {
	.name		= MODULENAME,
	.id_table	= rtl8169_pci_tbl,
	.probe		= rtl_init_one,
	.remove		= rtl_remove_one,
	.shutdown	= rtl_shutdown,
	.driver.pm	= NULL,
};

module_pci_driver(rtl8169_pci_driver);

