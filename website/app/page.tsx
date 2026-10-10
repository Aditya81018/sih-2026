import Link from "next/link";
import { TOTAL_FACTORY_COST_FORMATTED } from "./cost-list/data";

export default function Home() {
  return (
    <div className="min-h-screen bg-white text-gray-900 flex flex-col justify-between">
      {/* Top Header */}
      <header className="border-b border-gray-200 bg-white">
        <div className="max-w-5xl mx-auto px-4 sm:px-6 py-4 flex items-center justify-between">
          <div className="flex items-center gap-2">
            <span className="font-bold text-gray-900 text-sm">SIH 2026</span>
            <span className="text-gray-300">/</span>
            <span className="text-sm text-gray-600">Orbit V1 AGV &amp; Telescopic Mast Platform</span>
          </div>
          <nav className="flex items-center gap-4 text-xs font-medium text-gray-600">
            <Link href="/cost-list" className="hover:text-gray-900">
              Cost List
            </Link>
          </nav>
        </div>
      </header>

      {/* Main Content */}
      <main className="max-w-5xl mx-auto px-4 sm:px-6 py-12 flex-1 w-full">
        {/* Intro */}
        <div className="mb-8 pb-6 border-b border-gray-200">
          <div className="text-xs font-medium text-gray-500 uppercase tracking-wider mb-1">
            Smart India Hackathon 2026
          </div>
          <h1 className="text-3xl sm:text-4xl font-bold text-gray-900 mb-3">
            Orbit V1 AGV &amp; Telescopic Mast System
          </h1>
          <p className="text-sm sm:text-base text-gray-600 max-w-3xl leading-relaxed">
            Technical documentation, component specifications, and comprehensive Bill of Materials (BOM) cost breakdown covering the AGV mobile base, telescopic mast assembly, additive/subtractive manufacturing, and assembly labor.
          </p>
        </div>

        {/* Feature Cards Grid */}
        <div className="grid grid-cols-1 md:grid-cols-2 gap-4 mb-8">
          {/* Card 1: Cost List */}
          <Link
            href="/cost-list"
            className="block p-5 border border-gray-200 rounded hover:border-gray-400 bg-white transition"
          >
            <div className="flex items-center justify-between mb-2">
              <span className="text-xs font-mono font-semibold text-gray-600">/cost-list</span>
              <span className="text-xs font-mono font-bold text-gray-900">{TOTAL_FACTORY_COST_FORMATTED} Total</span>
            </div>
            <h2 className="text-lg font-bold text-gray-900 mb-1">
              Detailed Cost Breakdown &amp; BOM →
            </h2>
            <p className="text-xs text-gray-600 leading-relaxed mb-3">
              Full 19-item BOM across AGV mobile base (₹1,14,550), telescopic mast attachment (₹1,35,700), and system assembly &amp; calibration (₹33,000).
            </p>
            <span className="text-xs font-medium text-gray-900 underline">
              Open cost breakdown table
            </span>
          </Link>

          {/* Card 2: Technical Specifications */}
          <div className="p-5 border border-gray-200 rounded bg-white">
            <div className="flex items-center justify-between mb-2">
              <span className="text-xs font-mono font-semibold text-gray-600">HARDWARE SPECS</span>
              <span className="text-xs text-gray-500">10ft Stroke</span>
            </div>
            <h2 className="text-lg font-bold text-gray-900 mb-1">
              Hardware &amp; Fabrication
            </h2>
            <p className="text-xs text-gray-600 leading-relaxed mb-3">
              Large Format 3D Printed Composite Chassis (PETG-CF / ABS-CF) paired with 6061-T6 CNC aluminum mast channels and high-flex cable drag chains.
            </p>
            <span className="text-xs text-gray-500">
              Warehouse rack reach &amp; pallet transport
            </span>
          </div>
        </div>

        {/* System Subsystems Summary */}
        <div className="border border-gray-200 rounded p-5 bg-gray-50 text-xs">
          <h3 className="font-semibold text-gray-900 mb-2">System Architecture Summary</h3>
          <ul className="list-disc list-inside text-gray-700 space-y-1">
            <li><strong>AGV Mobile Base (Direct Cost ₹1,14,550):</strong> 24V 250W BLDC servo wheel drives, LiFePO4 battery with BMS, Raspberry Pi 4 + STM32 compute, Hikrobot downward QR camera, and LFAM composite 3D printed chassis.</li>
            <li><strong>Telescopic Mast &amp; Hand (Direct Cost ₹1,35,700):</strong> 10ft 9-layer CNC aluminum mast, NEMA closed-loop steppers, MyActuator RMD series BLDC joint servos, gripper mechanism, and Hikrobot hand camera.</li>
            <li><strong>Assembly &amp; Quality Control (₹33,000):</strong> Mechanical system integration, wiring harness termination, and ROS2/VDA5050 firmware calibration.</li>
            <li><strong>Total Factory Production Cost:</strong> <strong>₹2,83,250</strong></li>
          </ul>
        </div>
      </main>

      {/* Footer */}
      <footer className="border-t border-gray-200 py-6 text-center text-xs text-gray-500">
        Smart India Hackathon 2026 • Orbit V1 AGV &amp; Telescopic Mast System Documentation
      </footer>
    </div>
  );
}
