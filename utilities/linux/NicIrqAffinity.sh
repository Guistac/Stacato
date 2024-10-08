nicName="enp2s0f0"
targetCpuCore=2

# Find the number of the IRQ corresponding to the network interface
irqNumber=$(cat /proc/interrupts | grep -P $nicName | grep -oP '^\s*\K\d+(?=:)')

if [ -z "$irqNumber" ]; then
    echo "Could not find an interrupt associated with this network interface"
    exit
else
    echo "Setting affinity of irq #$irqNumber handling network interface $nicName to cpu core $targetCpuCore"

    # SWrite the CPU affinity of the IRQ
    echo $targetCpuCore | sudo tee /proc/irq/$irqNumber/smp_affinity_list > /dev/null

    echo "CPU affinity is now $(cat /proc/irq/$irqNumber/smp_affinity)"
fi