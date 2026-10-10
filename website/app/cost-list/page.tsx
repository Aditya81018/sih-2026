import type { Metadata } from "next";
import CostListClient from "./CostListClient";

export const metadata: Metadata = {
  title: "Orbit V1 Cost Breakdown | SIH 2026",
  description:
    "Comprehensive Bill of Materials (BOM) and manufacturing cost breakdown for the Orbit V1 AGV Mobile Base and Telescopic Mast System.",
  keywords: [
    "SIH 2026",
    "Orbit V1",
    "AGV Cost List",
    "Telescopic Mast",
    "LFAM 3D Printing",
    "Commercial Pricing Model",
    "BOM",
  ],
};

export default function CostListPage() {
  return <CostListClient />;
}
