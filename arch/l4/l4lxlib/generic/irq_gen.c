#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/irq.h>

#include <l4/sys/irq.h>
#include <l4/sys/icu.h>
#include <l4/sys/factory.h>
#include <l4/sys/task.h>
#include <l4/io/io.h>
#include <l4/re/env.h>

#include <asm/generic/cap_alloc.h>

#include <asm/l4lxapi/generic/irq_gen.h>

int l4x_alloc_irq_desc_data(int irq, l4_cap_idx_t icu)
{
	struct l4x_irq_desc_private *p;

	p = kzalloc(sizeof(*p), GFP_KERNEL);
	if (!p)
		return -ENOMEM;

	p->c.irq_cap = L4_INVALID_CAP;
	p->icu       = icu;

	return irq_set_chip_data(irq, p);
}


int l4lx_irq_set_type(struct irq_data *data, unsigned int type)
{
	unsigned int irq = data->irq;
	struct l4x_irq_desc_private *p;
	struct irq_desc *desc = irq_to_desc(data->irq);
	int r;

	if (unlikely(irq >= NR_IRQS))
		return -1;

	p = irq_get_chip_data(irq);
	if (!p)
		return -1;

	printk("L4IRQ: set irq type of %u to %x\n", irq, type);
	switch (type & IRQF_TRIGGER_MASK) {
		case IRQ_TYPE_EDGE_BOTH:
			p->trigger = L4_IRQ_F_BOTH_EDGE;
			desc->handle_irq = handle_edge_eoi_irq;
			break;
		case IRQ_TYPE_EDGE_RISING:
			p->trigger = L4_IRQ_F_POS_EDGE;
			desc->handle_irq = handle_edge_eoi_irq;
			break;
		case IRQ_TYPE_EDGE_FALLING:
			p->trigger = L4_IRQ_F_NEG_EDGE;
			desc->handle_irq = handle_edge_eoi_irq;
			break;
		case IRQ_TYPE_LEVEL_HIGH:
			p->trigger = L4_IRQ_F_LEVEL_HIGH;
			desc->handle_irq = handle_level_irq;
			break;
		case IRQ_TYPE_LEVEL_LOW:
			p->trigger = L4_IRQ_F_LEVEL_LOW;
			desc->handle_irq = handle_level_irq;
			break;
		default:
			p->trigger = L4_IRQ_F_NONE;
			break;
	};

	if (   l4_is_invalid_cap(p->c.irq_cap)
	    || l4_is_invalid_cap(p->icu))
		return 0;

	r = L4XV_FN_i(l4_error(l4_icu_set_mode(p->icu, irq, p->trigger)));
	if (r)
		pr_err("l4x-irq: l4-set-mode(%d) failed for IRQ %d: %d\n",
		       type, irq, r);

	return r;
}

void l4x_irq_release(l4_cap_idx_t irqcap)
{
	L4XV_FN_v(l4_task_unmap(L4_BASE_TASK_CAP,
	                        l4_obj_fpage(irqcap, 0, L4_FPAGE_RWX),
	                        L4_FP_ALL_SPACES));
}

l4_cap_idx_t l4x_irq_init(l4_cap_idx_t icu, unsigned irq, unsigned trigger,
                          char *tag)
{
	l4_cap_idx_t cap;

	if (l4_is_invalid_cap(cap = l4x_cap_alloc()))
		return L4_INVALID_CAP;

	if (L4XV_FN_i(l4_error(l4_icu_set_mode(icu, irq, trigger))) < 0) {
		pr_err("l4x: Failed to set type for IRQ %s:%d\n",
		       tag, irq);
		goto out1;
	}

	if (L4XV_FN_i(l4_error(l4_factory_create_irq(l4re_env()->factory,
	                       cap)))) {
		pr_err("l4x: Failed to create IRQ for IRQ %s:%d\n",
		       tag, irq);
		goto out1;
	}

	if (L4XV_FN_i(l4_error(l4_icu_bind(icu, irq, cap)))) {
		pr_err("l4x: Failed to bind IRQ %s:%d\n", tag, irq);
		goto out2;
	}

	return cap;

out2:
	l4x_irq_release(cap);
out1:
	l4x_cap_free(cap);
	return L4_INVALID_CAP;
}
