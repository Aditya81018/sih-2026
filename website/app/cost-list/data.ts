export interface CostItem {
  id: string;
  subsystemAxis: string;
  componentModel: string;
  qty: number;
  unitCost: number;
  unitCostFormatted: string;
  totalCost: number;
  totalCostFormatted: string;
  category: "Structure & Mechanics" | "Stepper Actuation" | "BLDC Servos" | "Electronics & Vision";
  badge: string;
  notes?: string;
}

export const COST_ITEMS: CostItem[] = [
  {
    id: "primary-mast-structure",
    subsystemAxis: "Primary Mast Structure",
    componentModel: "10ft 9-Layer Aluminum Telescopic Mast Hardware & Bearings",
    qty: 1,
    unitCost: 28000,
    unitCostFormatted: "₹28,000",
    totalCost: 28000,
    totalCostFormatted: "₹28,000",
    category: "Structure & Mechanics",
    badge: "Structure",
    notes: "Precision CNC milled 6061-T6 aluminum sections with low-friction linear bearings and safety stops."
  },
  {
    id: "secondary-hand-arm",
    subsystemAxis: "Secondary Hand Arm",
    componentModel: "3ft Horizontal Expansion Arm Aluminum Channel & Slides",
    qty: 1,
    unitCost: 12000,
    unitCostFormatted: "₹12,000",
    totalCost: 12000,
    totalCostFormatted: "₹12,000",
    category: "Structure & Mechanics",
    badge: "Extension Arm",
    notes: "3-foot stroke horizontal linear slide channel with dual linear rails for warehouse rack shelf reach."
  },
  {
    id: "primary-mast-motor",
    subsystemAxis: "Primary Mast Motor",
    componentModel: "NEMA 34 Closed-Loop Stepper + 10:1 Planetary Gearbox",
    qty: 1,
    unitCost: 14500,
    unitCostFormatted: "₹14,500",
    totalCost: 14500,
    totalCostFormatted: "₹14,500",
    category: "Stepper Actuation",
    badge: "Vertical Drive",
    notes: "High torque 8.5 N·m closed loop stepper drive with 10:1 precision planetary reduction for vertical lifting."
  },
  {
    id: "hand-expansion-motor",
    subsystemAxis: "Hand Expansion Motor",
    componentModel: "NEMA 23 Closed-Loop Stepper + 5:1 Planetary Gearbox",
    qty: 1,
    unitCost: 6200,
    unitCostFormatted: "₹6,200",
    totalCost: 6200,
    totalCostFormatted: "₹6,200",
    category: "Stepper Actuation",
    badge: "Linear Drive",
    notes: "Compact closed-loop stepper motor driving rack-and-pinion horizontal arm reach."
  },
  {
    id: "arm-pitch-motor",
    subsystemAxis: "Arm Pitch Motor (Top Base)",
    componentModel: "MyActuator RMD-X8-120 BLDC Servo + Harmonic Reducer",
    qty: 1,
    unitCost: 26000,
    unitCostFormatted: "₹26,000",
    totalCost: 26000,
    totalCostFormatted: "₹26,000",
    category: "BLDC Servos",
    badge: "Joint Pitch",
    notes: "Integrated BLDC servo motor with built-in zero-backlash harmonic drive for high-load arm pitch positioning."
  },
  {
    id: "wrist-pitch-servo",
    subsystemAxis: "Wrist Pitch Servo (Hand)",
    componentModel: "MyActuator RMD-X6-60 BLDC Frameless Servo Joint",
    qty: 1,
    unitCost: 16500,
    unitCostFormatted: "₹16,500",
    totalCost: 16500,
    totalCostFormatted: "₹16,500",
    category: "BLDC Servos",
    badge: "Wrist Pitch",
    notes: "Compact hollow-shaft BLDC robotic actuator providing high power density and CAN bus control."
  },
  {
    id: "wrist-roll-servo",
    subsystemAxis: "Wrist Roll Servo (Hand)",
    componentModel: "MyActuator RMD-X4-36 BLDC Frameless Servo Joint",
    qty: 1,
    unitCost: 12500,
    unitCostFormatted: "₹12,500",
    totalCost: 12500,
    totalCostFormatted: "₹12,500",
    category: "BLDC Servos",
    badge: "Wrist Roll",
    notes: "Ultra-compact continuous rotation robotic joint for accurate end-effector orientation."
  },
  {
    id: "gripper-actuator",
    subsystemAxis: "Gripper Actuator (Hand)",
    componentModel: "NEMA 17 Lead Screw Stepper Motor & Fingers",
    qty: 1,
    unitCost: 3200,
    unitCostFormatted: "₹3,200",
    totalCost: 3200,
    totalCostFormatted: "₹3,200",
    category: "Stepper Actuation",
    badge: "End Effector",
    notes: "Integrated lead-screw stepper mechanism with silicone-padded adaptive parallel fingers."
  },
  {
    id: "mast-controller-safety",
    subsystemAxis: "Mast Controller & Safety",
    componentModel: "STM32F407 Board + Limit Switches + Driver Shields",
    qty: 1,
    unitCost: 6800,
    unitCostFormatted: "₹6,800",
    totalCost: 6800,
    totalCostFormatted: "₹6,800",
    category: "Electronics & Vision",
    badge: "Safety & Logic",
    notes: "Embedded 168MHz ARM Cortex-M4 real-time controller, optocoupled optical endstops & E-stop loop."
  },
  {
    id: "vision-camera",
    subsystemAxis: "Vision Camera",
    componentModel: "Hikrobot Industrial Hand Camera & Mounts",
    qty: 1,
    unitCost: 12500,
    unitCostFormatted: "₹12,500",
    totalCost: 12500,
    totalCostFormatted: "₹12,500",
    category: "Electronics & Vision",
    badge: "Perception",
    notes: "Global shutter GigE / USB3 industrial color camera with adjustable focal lens for SKU barcoding and shelf fiducials."
  },
  {
    id: "basket-drag-chains",
    subsystemAxis: "Basket & Drag Chains",
    componentModel: "Payload Basket + High-Flex Cable Drag Chains",
    qty: 1,
    unitCost: 5500,
    unitCostFormatted: "₹5,500",
    totalCost: 5500,
    totalCostFormatted: "₹5,500",
    category: "Structure & Mechanics",
    badge: "Cable Mgmt",
    notes: "Reinforced carbon-fiber basket frame with bend-radius limited nylon drag chains for continuous cycling."
  }
];

export const TOTAL_COST = 143700;
export const TOTAL_COST_FORMATTED = "₹1,43,700";
export const SUBSYSTEM_TITLE = "MAST SUBSYSTEM TOTAL";

export const CATEGORY_SUMMARIES = [
  {
    category: "BLDC Servos",
    cost: 55000,
    costFormatted: "₹55,000",
    percentage: 38.3,
    color: "#3b82f6", // blue-500
    count: 3
  },
  {
    category: "Structure & Mechanics",
    cost: 45500,
    costFormatted: "₹45,500",
    percentage: 31.7,
    color: "#10b981", // emerald-500
    count: 3
  },
  {
    category: "Stepper Actuation",
    cost: 23900,
    costFormatted: "₹23,900",
    percentage: 16.6,
    color: "#f59e0b", // amber-500
    count: 3
  },
  {
    category: "Electronics & Vision",
    cost: 19300,
    costFormatted: "₹19,300",
    percentage: 13.4,
    color: "#8b5cf6", // purple-500
    count: 2
  }
];

export const RAW_CSV = `Subsystem / Axis,Component Model / Specification,Qty,Est. Unit Cost (INR),Total Cost (INR)
Primary Mast Structure,10ft 9-Layer Aluminum Telescopic Mast Hardware & Bearings,1,"₹28,000","₹28,000"
Secondary Hand Arm,3ft Horizontal Expansion Arm Aluminum Channel & Slides,1,"₹12,000","₹12,000"
Primary Mast Motor,NEMA 34 Closed-Loop Stepper + 10:1 Planetary Gearbox,1,"₹14,500","₹14,500"
Hand Expansion Motor,NEMA 23 Closed-Loop Stepper + 5:1 Planetary Gearbox,1,"₹6,200","₹6,200"
Arm Pitch Motor (Top Base),MyActuator RMD-X8-120 BLDC Servo + Harmonic Reducer,1,"₹26,000","₹26,000"
Wrist Pitch Servo (Hand),MyActuator RMD-X6-60 BLDC Frameless Servo Joint,1,"₹16,500","₹16,500"
Wrist Roll Servo (Hand),MyActuator RMD-X4-36 BLDC Frameless Servo Joint,1,"₹12,500","₹12,500"
Gripper Actuator (Hand),NEMA 17 Lead Screw Stepper Motor & Fingers,1,"₹3,200","₹3,200"
Mast Controller & Safety,STM32F407 Board + Limit Switches + Driver Shields,1,"₹6,800","₹6,800"
Vision Camera,Hikrobot Industrial Hand Camera & Mounts,1,"₹12,500","₹12,500"
Basket & Drag Chains,Payload Basket + High-Flex Cable Drag Chains,1,"₹5,500","₹5,500"
MAST SUBSYSTEM TOTAL,,,,"₹1,43,700"`;
