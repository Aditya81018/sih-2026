export interface CostItem {
  id: string;
  systemModule: "AGV Mobile Base" | "AGV Manufacturing" | "Telescopic Mast" | "Mast Manufacturing" | "Assembly & Overheads";
  itemDescription: string;
  specification?: string;
  qty: number;
  unitCost: number;
  unitCostFormatted: string;
  subtotal: number;
  subtotalFormatted: string;
  category: "AGV Base" | "Mast System" | "Manufacturing & Assembly";
}

export const COST_ITEMS: CostItem[] = [
  // Subsystem 1: AGV Mobile Base Unit & Manufacturing
  {
    id: "agv-wheel-motors",
    systemModule: "AGV Mobile Base",
    itemDescription: "24V 250W BLDC Servo Drive Wheel Modules (Pair)",
    specification: "ZLLG45ASM200 4.5\" Hub Motors with integrated hall encoders",
    qty: 2,
    unitCost: 8700,
    unitCostFormatted: "₹8,700",
    subtotal: 17400,
    subtotalFormatted: "₹17,400",
    category: "AGV Base",
  },
  {
    id: "agv-motor-driver",
    systemModule: "AGV Mobile Base",
    itemDescription: "Dual BLDC Motor Controller (24V 10A CAN/PWM)",
    specification: "AQMD2410BL-K3 dual channel closed-loop driver",
    qty: 1,
    unitCost: 5500,
    unitCostFormatted: "₹5,500",
    subtotal: 5500,
    subtotalFormatted: "₹5,500",
    category: "AGV Base",
  },
  {
    id: "agv-battery-bms",
    systemModule: "AGV Mobile Base",
    itemDescription: "24V 30Ah LiFePO4 Prismatic Battery Pack + Daly 8S BMS",
    specification: "High discharge prismatic cells with active balancing BMS",
    qty: 1,
    unitCost: 16900,
    unitCostFormatted: "₹16,900",
    subtotal: 16900,
    subtotalFormatted: "₹16,900",
    category: "AGV Base",
  },
  {
    id: "agv-casters",
    systemModule: "AGV Mobile Base",
    itemDescription: "75mm Heavy Duty Polyurethane Swivel Casters (Set of 4)",
    specification: "Industrial PU caster wheels with dual ball bearings",
    qty: 1,
    unitCost: 3150,
    unitCostFormatted: "₹3,150",
    subtotal: 3150,
    subtotalFormatted: "₹3,150",
    category: "AGV Base",
  },
  {
    id: "agv-compute",
    systemModule: "AGV Mobile Base",
    itemDescription: "Raspberry Pi 4 (4GB) + STM32 Motion Controller",
    specification: "Embedded ROS2 navigation host & real-time motor controller",
    qty: 1,
    unitCost: 7000,
    unitCostFormatted: "₹7,000",
    subtotal: 7000,
    subtotalFormatted: "₹7,000",
    category: "AGV Base",
  },
  {
    id: "agv-vision",
    systemModule: "AGV Mobile Base",
    itemDescription: "Downward Industrial QR Camera & 24V Ring Light",
    specification: "Hikrobot QR barcode global shutter camera with illumination",
    qty: 1,
    unitCost: 13700,
    unitCostFormatted: "₹13,700",
    subtotal: 13700,
    subtotalFormatted: "₹13,700",
    category: "AGV Base",
  },
  {
    id: "agv-safety-harness",
    systemModule: "AGV Mobile Base",
    itemDescription: "Safety Bumpers, Relays, Ultrasonics, IMU & Wiring Harness",
    specification: "Tactile emergency stops, 9-DOF IMU, wiring loom and relays",
    qty: 1,
    unitCost: 13400,
    unitCostFormatted: "₹13,400",
    subtotal: 13400,
    subtotalFormatted: "₹13,400",
    category: "AGV Base",
  },
  {
    id: "agv-chassis-lfam",
    systemModule: "AGV Manufacturing",
    itemDescription: "LFAM 3D Printed Composite Chassis (PETG-CF / ABS-CF)",
    specification: "Large Format Additive Manufacturing carbon-fiber polymer chassis",
    qty: 1,
    unitCost: 37500,
    unitCostFormatted: "₹37,500",
    subtotal: 37500,
    subtotalFormatted: "₹37,500",
    category: "Manufacturing & Assembly",
  },

  // Subsystem 2: Telescopic Mast & Hand Attachment
  {
    id: "mast-stepper-lift",
    systemModule: "Telescopic Mast",
    itemDescription: "NEMA 34 Closed-Loop Stepper + 10:1 Planetary Gearbox (Primary Extension)",
    specification: "8.5 N·m torque closed-loop vertical lift actuator",
    qty: 1,
    unitCost: 14500,
    unitCostFormatted: "₹14,500",
    subtotal: 14500,
    subtotalFormatted: "₹14,500",
    category: "Mast System",
  },
  {
    id: "mast-stepper-extension",
    systemModule: "Telescopic Mast",
    itemDescription: "NEMA 23 Closed-Loop Stepper + 5:1 Planetary (Hand Extension)",
    specification: "Horizontal linear extension arm gearmotor",
    qty: 1,
    unitCost: 6200,
    unitCostFormatted: "₹6,200",
    subtotal: 6200,
    subtotalFormatted: "₹6,200",
    category: "Mast System",
  },
  {
    id: "mast-servo-pitch",
    systemModule: "Telescopic Mast",
    itemDescription: "MyActuator RMD-X8-120 BLDC Servo Actuator (Arm Pitch)",
    specification: "Harmonic reducer frameless joint servo with CAN bus",
    qty: 1,
    unitCost: 26000,
    unitCostFormatted: "₹26,000",
    subtotal: 26000,
    subtotalFormatted: "₹26,000",
    category: "Mast System",
  },
  {
    id: "mast-servos-wrist",
    systemModule: "Telescopic Mast",
    itemDescription: "MyActuator RMD-X6-60 & RMD-X4-36 Servos (Wrist Joints)",
    specification: "Dual compact BLDC wrist pitch & roll frameless joints",
    qty: 1,
    unitCost: 29000,
    unitCostFormatted: "₹29,000",
    subtotal: 29000,
    subtotalFormatted: "₹29,000",
    category: "Mast System",
  },
  {
    id: "mast-gripper",
    systemModule: "Telescopic Mast",
    itemDescription: "NEMA 17 Lead Screw Stepper & Gripper Mechanism",
    specification: "Integrated lead screw mechanism with parallel fingers",
    qty: 1,
    unitCost: 3200,
    unitCostFormatted: "₹3,200",
    subtotal: 3200,
    subtotalFormatted: "₹3,200",
    category: "Mast System",
  },
  {
    id: "mast-controller",
    systemModule: "Telescopic Mast",
    itemDescription: "Dedicated Mast STM32 Controller, Limit Switches & CAN Shields",
    specification: "Embedded controller, endstops, and optocoupled driver board",
    qty: 1,
    unitCost: 6800,
    unitCostFormatted: "₹6,800",
    subtotal: 6800,
    subtotalFormatted: "₹6,800",
    category: "Mast System",
  },
  {
    id: "mast-camera",
    systemModule: "Telescopic Mast",
    itemDescription: "Hikrobot Hand Vision Camera & Mounts",
    specification: "Industrial hand-mounted perception camera with GigE/USB3",
    qty: 1,
    unitCost: 12500,
    unitCostFormatted: "₹12,500",
    subtotal: 12500,
    subtotalFormatted: "₹12,500",
    category: "Mast System",
  },
  {
    id: "mast-structure-cnc",
    systemModule: "Mast Manufacturing",
    itemDescription: "10ft 9-Layer Telescopic Structural Channels & CNC Bearings",
    specification: "Precision milled 6061-T6 aluminum sections and slides",
    qty: 1,
    unitCost: 32000,
    unitCostFormatted: "₹32,000",
    subtotal: 32000,
    subtotalFormatted: "₹32,000",
    category: "Manufacturing & Assembly",
  },
  {
    id: "mast-drag-chains",
    systemModule: "Mast Manufacturing",
    itemDescription: "High-Flex Energy Cable Carrier Drag Chains & Brackets",
    specification: "Continuous bend-radius limited nylon cable carriers",
    qty: 1,
    unitCost: 5500,
    unitCostFormatted: "₹5,500",
    subtotal: 5500,
    subtotalFormatted: "₹5,500",
    category: "Manufacturing & Assembly",
  },

  // Subsystem 3: Assembly, Testing & Overheads
  {
    id: "assembly-labor",
    systemModule: "Assembly & Overheads",
    itemDescription: "System Integration, Harness Wiring & Mechanical Assembly Labor",
    specification: "Mechanical assembly, wire harness termination and quality check",
    qty: 1,
    unitCost: 18000,
    unitCostFormatted: "₹18,000",
    subtotal: 18000,
    subtotalFormatted: "₹18,000",
    category: "Manufacturing & Assembly",
  },
  {
    id: "software-calibration",
    systemModule: "Assembly & Overheads",
    itemDescription: "ROS2/VDA5050 Calibration, EKF Tuning & Safety Testing",
    specification: "Firmware flashing, sensor fusion tuning, and factory validation",
    qty: 1,
    unitCost: 15000,
    unitCostFormatted: "₹15,000",
    subtotal: 15000,
    subtotalFormatted: "₹15,000",
    category: "Manufacturing & Assembly",
  },
];

export const TOTAL_FACTORY_COST = 283250;
export const TOTAL_FACTORY_COST_FORMATTED = "₹2,83,250";

export const PRICING_MODEL = {
  factoryCost: 283250,
  factoryCostFormatted: "₹2,83,250",
  agvDirectCost: 114550,
  agvDirectCostFormatted: "₹1,14,550",
  mastDirectCost: 135700,
  mastDirectCostFormatted: "₹1,35,700",
  assemblyTestingCost: 33000,
  assemblyTestingCostFormatted: "₹33,000",
  prototypeRate: {
    margin: "25%",
    calculatedPrice: 377667,
    calculatedFormatted: "₹3,77,600",
    range: "₹3,75,000 – ₹3,95,000",
    usdRange: "~$4,500 USD",
    notes: "Ideal for SIH 2026 presentation metrics, academic pilot testing, or grant budget allocation.",
  },
  commercialPrice: {
    margin: "40%",
    calculatedPrice: 472083,
    calculatedFormatted: "₹4,72,000",
    range: "₹4,50,000 – ₹4,80,000",
    usdRange: "~$5,500 – $5,800 USD",
    notes: "Margin coverage for warranty support, WMS integration software, and distributor commissions.",
  },
};

export const MODULE_SUMMARIES = [
  {
    module: "AGV Mobile Base",
    cost: 77050,
    costFormatted: "₹77,050",
    percentage: 27.2,
    count: 7,
  },
  {
    module: "Telescopic Mast",
    cost: 98200,
    costFormatted: "₹98,200",
    percentage: 34.7,
    count: 7,
  },
  {
    module: "AGV Manufacturing",
    cost: 37500,
    costFormatted: "₹37,500",
    percentage: 13.2,
    count: 1,
  },
  {
    module: "Mast Manufacturing",
    cost: 37500,
    costFormatted: "₹37,500",
    percentage: 13.2,
    count: 2,
  },
  {
    module: "Assembly & Overheads",
    cost: 33000,
    costFormatted: "₹33,000",
    percentage: 11.7,
    count: 2,
  },
];

export const RAW_CSV = `System Module,Item Description,Qty,Unit Cost (INR),Subtotal (INR)
AGV Mobile Base,24V 250W BLDC Servo Drive Wheel Modules (Pair),2,"₹8,700","₹17,400"
AGV Mobile Base,Dual BLDC Motor Controller (24V 10A CAN/PWM),1,"₹5,500","₹5,500"
AGV Mobile Base,24V 30Ah LiFePO4 Prismatic Battery Pack + Daly 8S BMS),1,"₹16,900","₹16,900"
AGV Mobile Base,75mm Heavy Duty Polyurethane Swivel Casters (Set of 4),1,"₹3,150","₹3,150"
AGV Mobile Base,Raspberry Pi 4 (4GB) + STM32 Motion Controller,1,"₹7,000","₹7,000"
AGV Mobile Base,Downward Industrial QR Camera & 24V Ring Light,1,"₹13,700","₹13,700"
AGV Mobile Base,Safety Bumpers, Relays, Ultrasonics, IMU & Wiring Harness,1,"₹13,400","₹13,400"
AGV Manufacturing,LFAM 3D Printed Composite Chassis (PETG-CF / ABS-CF),1,"₹37,500","₹37,500"
Telescopic Mast,NEMA 34 Closed-Loop Stepper + 10:1 Planetary Gearbox (Primary Extension),1,"₹14,500","₹14,500"
Telescopic Mast,NEMA 23 Closed-Loop Stepper + 5:1 Planetary (Hand Extension),1,"₹6,200","₹6,200"
Telescopic Mast,MyActuator RMD-X8-120 BLDC Servo Actuator (Arm Pitch),1,"₹26,000","₹26,000"
Telescopic Mast,MyActuator RMD-X6-60 & RMD-X4-36 Servos (Wrist Joints),1,"₹29,000","₹29,000"
Telescopic Mast,NEMA 17 Lead Screw Stepper & Gripper Mechanism,1,"₹3,200","₹3,200"
Telescopic Mast,Dedicated Mast STM32 Controller, Limit Switches & CAN Shields,1,"₹6,800","₹6,800"
Telescopic Mast,Hikrobot Hand Vision Camera & Mounts,1,"₹12,500","₹12,500"
Mast Manufacturing,10ft 9-Layer Telescopic Structural Channels & CNC Bearings,1,"₹32,000","₹32,000"
Mast Manufacturing,High-Flex Energy Cable Carrier Drag Chains & Brackets,1,"₹5,500","₹5,500"
Assembly & Overheads,System Integration, Harness Wiring & Mechanical Assembly Labor,1,"₹18,000","₹18,000"
Assembly & Overheads,ROS2/VDA5050 Calibration, EKF Tuning & Safety Testing,1,"₹15,000","₹15,000"
TOTAL FACTORY PRODUCTION COST,,,,₹2,83,250`;
