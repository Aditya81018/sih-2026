import type { Metadata } from "next";
import CostListClient from "./CostListClient";

export const metadata: Metadata = {
  title: "Mast Subsystem Cost List | SIH 2026",
  description:
    "Detailed Bill of Materials (BOM) and cost breakdown for the 10ft 9-Layer Mast, Secondary Hand Arm, Stepper Actuation, BLDC Frameless Servos, and Vision Subsystem.",
  keywords: [
    "SIH 2026",
    "Mast Subsystem",
    "Cost List",
    "BOM",
    "Telescopic Mast",
    "MyActuator BLDC",
    "Robotic Arm",
    "Smart India Hackathon",
  ],
};

export default function CostListPage() {
  return <CostListClient />;
}
